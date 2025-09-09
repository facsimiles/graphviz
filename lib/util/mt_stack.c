#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/alloc.h>
#include <util/hazard.h>
#include <util/mt_stack.h>

#define LOAD(ptr) atomic_load_explicit((ptr), memory_order_acquire)
#define STORE(ptr, val)                                                        \
  atomic_store_explicit((ptr), (val), memory_order_release)
#define CAS(target, expected, desired)                                         \
  atomic_compare_exchange_strong_explicit((target), (expected), (desired),     \
                                          memory_order_acq_rel,                \
                                          memory_order_acquire)

typedef uintptr_t ref_t;

#define BLOCK_SIZE ((ref_t)4096)

/// The use of `_Alignas` on the first member is actually to align the
/// containing struct. This is apparently the recommended way to do this in C
/// where, unlike C++, it is not possible to use alignment constraints directly
/// on the struct definition.
typedef struct block {
  _Alignas(BLOCK_SIZE) struct block *_Atomic previous;
  void *_Atomic item[(BLOCK_SIZE - sizeof(uintptr_t)) / sizeof(void *)];
} block_t;

static block_t *ref2block(ref_t ref) {
  return (block_t *)(ref & ~(BLOCK_SIZE - 1));
}

/// derive a reference to the last item in a block
static ref_t block2back(const block_t *block) {
  if (block == NULL) {
    return 0;
  }
#if 0
  // This is what we would like to return. But, although `&block->item[…]` can
  // be computed as a pointer offset, it technically dereferences `block` which
  // our caller may not have hazarded. So we instead use a safer formulation.
  return (ref_t)&block->item[sizeof(block->item) / sizeof(block->item[0]) - 1];
#else
  return (ref_t)((uintptr_t)block + offsetof(block_t, item) +
                 sizeof(block->item) - sizeof(block->item[0]));
#endif
}

/// does this reference point to an item (as opposed to the previous pointer)?
static bool ref_is_item(ref_t ref) {
  return (ref & (BLOCK_SIZE - 1)) >= offsetof(block_t, item);
}

static size_t ref2index(ref_t ref) {
  assert(ref_is_item(ref));
  return ((ref & (BLOCK_SIZE - 1)) - offsetof(block_t, item)) / sizeof(void *);
}

static ref_t index2ref(const block_t *block, size_t index) {
  assert(index < sizeof(block->item) / sizeof(block->item[0]));
  return (ref_t)&block->item[index];
}

static bool block_is_empty(ref_t ref) { return !ref_is_item(ref); }

static bool block_is_full(ref_t ref) {
  if (ref == 0) {
    return true;
  }
  const block_t *block = ref2block(ref);
  const size_t index = ref2index(ref);
  return index == sizeof(block->item) / sizeof(block->item[0]) - 1;
}

struct mt_stack {
  hazard_set_t *hazards;
  _Atomic ref_t top;
};

mt_stack_t *gv_mt_stack_new(size_t threads) {
  assert(threads > 0);

  mt_stack_t *const stack = gv_alloc(sizeof(*stack));
  stack->hazards = gv_hazard_set_new(threads, gv_aligned_free);

  return stack;
}

void gv_mt_stack_push(mt_stack_t *stack, size_t thread_id, void *item) {
  assert(stack != NULL);
  assert(item != NULL);

  ref_t top = LOAD(&stack->top);
retry:

  if (block_is_full(top)) {
    // the current block is full, so we need to allocate a new one

    block_t *const block = gv_aligned_alloc(BLOCK_SIZE, sizeof(*block));
    STORE(&block->item[0], item);
    block_t *const previous = ref2block(top);
    STORE(&block->previous, previous);

    const ref_t new_top = index2ref(block, 0);
    if (!CAS(&stack->top, &top, new_top)) {
      // failed
      gv_aligned_free(block);
      goto retry;
    }

    return;
  }

  block_t *const block = ref2block(top);

  // protect our upcoming access to the top block
  gv_hazard(stack->hazards, thread_id, block);
  {
    // we need to do something similar to double-checked locking here to confirm
    // no one else pushed or popped in the time since we read the top and before
    // we flagged our intent to dereference
    const ref_t reread = LOAD(&stack->top);
    if (reread != top) {
      // someone else successfully raced us
      gv_unhazard(stack->hazards, thread_id, block);
      top = reread;
      goto retry;
    }
  }

  // push our new item
  const size_t index = ref2index(top) + 1;
  if (!CAS(&block->item[index], &(void *){NULL}, item)) {
    // another pusher beat us
    gv_unhazard(stack->hazards, thread_id, block);
    top = LOAD(&stack->top);
    goto retry;
  }

  // update the top pointer
  const ref_t new_top = index2ref(block, index);
  if (!CAS(&stack->top, &top, new_top)) {
    // a popper beat us; we need to undo our push progress

    // this CAS should never fail because we “own” the stack slot one beyond the
    // initial top by virtue of having successfully CAS-ed into it above
    const bool r = CAS(&block->item[index], &(void *){item}, NULL);
    assert(r && "conflicting uses of N+1 stack slot");
    (void)r;

    gv_unhazard(stack->hazards, thread_id, block);
    goto retry;
  }

  gv_unhazard(stack->hazards, thread_id, block);
}

void *gv_mt_stack_pop(mt_stack_t *stack, size_t thread_id) {
  assert(stack != NULL);

  ref_t top = LOAD(&stack->top);
retry:
  if (top == 0) {
    // the stack is empty
    return NULL;
  }

  block_t *const block = ref2block(top);

  // protect our upcoming access to the top block
  gv_hazard(stack->hazards, thread_id, block);
  {
    // we need to do something similar to double-checked locking here to confirm
    // no one else pushed or popped in the time since we read the top and before
    // we flagged our intent to dereference
    const ref_t reread = LOAD(&stack->top);
    if (reread != top) {
      // someone else successfully raced us
      gv_unhazard(stack->hazards, thread_id, block);
      top = reread;
      goto retry;
    }
  }

  if (block_is_empty(top)) {
    // we need to pop to the previous one
    block_t *const previous = LOAD(&block->previous);
    const ref_t new_top = block2back(previous);
    const bool r = CAS(&stack->top, &top, new_top);
    gv_unhazard(stack->hazards, thread_id, block);
    if (r) {
      // succeeded
      gv_reclaim(stack->hazards, thread_id, block);
    }
    // whether we succeeded or failed, we have a new stack top so need to
    // restart our algorithm
    goto retry;
  }

  const size_t index = ref2index(top);

  // Mark the top item as popped. It is important to do this first to avoid ABA
  // problems. For background on the ABA problem:
  //   • https://en.wikipedia.org/wiki/ABA_problem
  //   • R. Kent Treiber, “Systems Programming: Coping with Parallelism” IBM
  //     1986
  //   • “IBM System/370 Extended Architecture, Principles of Operation” 1983
  const uintptr_t new_top = top - sizeof(block->item[index]);
  if (!CAS(&stack->top, &top, new_top)) {
    // a pusher or popper beat us
    gv_unhazard(stack->hazards, thread_id, block);
    goto retry;
  }

  // pop the top item
  void *popped = LOAD(&block->item[index]);
  assert(popped != NULL && "null pointer was present in stack");
  {
    const bool r = CAS(&block->item[index], &popped, NULL);
    assert(r && "unanticipated race in popping stack items");
    (void)r;
  }

  gv_unhazard(stack->hazards, thread_id, block);

  return popped;
}

void gv_mt_stack_free(mt_stack_t *stack) {
  if (stack != NULL) {
    // discard any remaining entries
    while (gv_mt_stack_pop(stack, 0) != NULL) {
      // empty
    }

    gv_hazard_set_free(stack->hazards);
  }
  free(stack);
}
