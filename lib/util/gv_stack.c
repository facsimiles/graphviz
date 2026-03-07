#ifndef NO_CONFIG // defined by test_stack.c
#include "config.h"
#endif

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/alloc.h>
#include <util/asan.h>
#include <util/exit.h>
#include <util/gv_math.h>
#include <util/gv_stack-private.h>
#include <util/prisize_t.h>

static int gv_stack_try_reserve_(gv_stack_t_ *stack, size_t capacity,
                                 size_t item_size);

void gv_stack_increase_capacity_(gv_stack_t_ *stack, size_t item_size) {
  const size_t stack_capacity = gv_stack_capacity_(stack, item_size);
  const size_t c = stack_capacity == 0 ? 1 : (stack_capacity * 2);
  gv_stack_reserve_(stack, c, item_size);
}

int gv_stack_try_increase_capacity_(gv_stack_t_ *stack, size_t item_size) {
  const size_t stack_capacity = gv_stack_capacity_(stack, item_size);
  const size_t c = stack_capacity == 0 ? 1 : (stack_capacity * 2);
  return gv_stack_try_reserve_(stack, c, item_size);
}

// Returns 0 if no error, otherwise an errno
static int gv_stack_try_reserve_(gv_stack_t_ *stack, size_t capacity,
                                 size_t item_size) {
  assert(stack != NULL);

  // if we can already fit enough items, nothing to do
  const size_t stack_capacity = gv_stack_capacity_(stack, item_size);
  if (capacity <= stack_capacity) {
    return 0;
  }

  assert(stack->next_slot == stack->limit);
  assert((char *)stack->base <= (char *)stack->limit);
  size_t limit_bytes = (size_t)((char *)stack->limit - (char *)stack->base);

  // will the arithmetic below overflow?
  assert(0 < capacity);
  assert(0 < item_size);
  if (SIZE_MAX / capacity < item_size) {
    return EOVERFLOW;
  }

  size_t capacity_bytes = capacity * item_size;
  void *const base = realloc(stack->base, capacity_bytes);
  if (base == NULL) {
    return ENOMEM;
  }

  // zero the new memory
  {
    void *const new = ((char *)base) + limit_bytes;
    const size_t new_bytes = capacity_bytes - limit_bytes;
    memset(new, 0, new_bytes);

    // poison the new (conceptually unallocated) memory
    ASAN_POISON(new, limit_bytes);
  }

  // previously, stack->next_slot == stack->limit
  stack->next_slot = ((char *)base) + limit_bytes;
  stack->base = base;
  stack->limit = ((char *)base) + capacity_bytes;
  return 0;
}

void gv_stack_clear_(gv_stack_t_ *stack, size_t item_size) {
  assert(stack != NULL);

  if (stack->base == NULL) {
    assert(stack->next_slot == NULL);
    return;
  }
  char *cursor = (char *)stack->base;
  char *limit = (char *)stack->next_slot;
  for (; cursor < limit; cursor += item_size) {
    ASAN_POISON((void *)cursor, item_size);
  }
  stack->next_slot = stack->base;
}

void gv_stack_reserve_(gv_stack_t_ *stack, size_t capacity, size_t item_size) {
  const int err = gv_stack_try_reserve_(stack, capacity, item_size);
  if (err != 0) {
    fprintf(stderr,
            "failed to reserve %" PRISIZE_T " elements of size %" PRISIZE_T
            " bytes: %s\n",
            capacity, item_size, strerror(err));
    graphviz_exit(EXIT_FAILURE);
  }
}

void gv_stack_sort_(gv_stack_t_ *stack, int (*cmp)(const void *, const void *),
                    size_t item_size) {
  assert(stack != NULL);
  assert(cmp != NULL);

  size_t size = gv_stack_size_(stack, item_size);
  if (0 < size) {
    qsort(stack->base, size, item_size, cmp);
  }
}

static void exchange(void *a, void *b, size_t size) {
  assert(a != NULL);
  assert(b != NULL);

  // do a byte-by-byte swap of the two objects
  char *x = a;
  char *y = b;
  for (size_t i = 0; i < size; ++i) {
    SWAP(&x[i], &y[i]);
  }
}

void gv_stack_reverse_(gv_stack_t_ *stack, size_t item_size) {
  assert(stack != NULL);

  if (stack->base == NULL) {
    assert(stack->next_slot == NULL);
    return;
  }
  // move from the outside inwards, swapping elements
  char *a = (char *)stack->base;
  char *b = ((char *)stack->next_slot) - item_size;
  while (a < b) {
    exchange(a, b, item_size);
    a += item_size;
    b -= item_size;
  }
}

void gv_stack_free_(gv_stack_t_ *stack) {
  assert(stack != NULL);
  free(stack->base);
  *stack = (gv_stack_t_){0};
}

void gv_stack_pop_back_(gv_stack_t_ *stack, void *into, size_t item_size) {
  assert(stack != NULL);
  assert(stack->next_slot != stack->base);
  assert(stack->base != NULL);
  char *new_next_slot = ((char *)stack->next_slot) - item_size;
  assert((char *)stack->base <= new_next_slot);
  memcpy(into, new_next_slot, item_size);
  stack->next_slot = (void *)new_next_slot;
  ASAN_POISON(new_next_slot, item_size);
}
