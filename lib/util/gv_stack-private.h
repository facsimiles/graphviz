/// @file
/// @brief internal implementation details of stack.h
/// @ingroup cgraph_utils
///
/// Everything in this header should be considered “private” in the sense that
/// it should not be called except by macros in stack.h.

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/api.h>
#include <util/asan.h>

#ifdef __cplusplus
extern "C" {
#endif

/// generic stack, agnostic to the stack item type
///
/// There is no way to know the size of stack items from this structure alone.
/// Stack item size is expected to be supplied externally.
typedef struct {
  void *base; ///< start of the allocation for backing memory
  ///< (base == NULL && limit == NULL) || (base != NULL && limit > base)
  void *next_slot; /// pointer to next spot to use in backing memory.
  ///< (limit == NULL && next_slot == NULL)
  ///  || (limit != NULL && next_slot <= limit)
  void *limit; /// pointer just past available memory in backing memory.
} gv_stack_t_;

/// get the number of elements in a stack
///
/// @param stack Stack to inspect
/// @param stack item_size Byte size of each stack item
/// @return Size of the stack
static inline size_t gv_stack_size_(const gv_stack_t_ *stack,
                                    size_t item_size) {
  assert((char *)stack->base <= (char *)stack->next_slot);
  return (size_t)(((char *)(stack->next_slot)) - ((char *)(stack->base))) /
         item_size;
}

/// test for stack emptiness
///
/// @param stack Stack to inspect
/// @return Size of the stack
static inline size_t gv_stack_is_empty_(const gv_stack_t_ *stack) {
  return stack->next_slot == stack->base;
}

/// get the capacity of a stack
///
/// @param stack Stack to inspect
/// @param stack item_size Byte size of each stack item
/// @return Size of the stack
static inline size_t gv_stack_capacity_(const gv_stack_t_ *stack,
                                        size_t item_size) {
  assert((char *)stack->base <= (char *)stack->limit);
  return (size_t)((((char *)(stack->limit)) - ((char *)(stack->base))) /
                  item_size);
}

/// get the remaining capacity of a stack
///
/// @param stack Stack to inspect
/// @param stack item_size Byte size of each stack item
/// @return Size of the stack
static inline size_t gv_stack_available_capacity_(const gv_stack_t_ *stack,
                                                  size_t item_size) {
  assert((char *)stack->next_slot <= (char *)stack->limit);
  return (size_t)((((char *)(stack->limit)) - ((char *)(stack->next_slot))) /
                  item_size);
}

/// test whether stack can take another item without allocation.
///
/// @param stack Stack to inspect
/// @param stack item_size Byte size of each stack item
/// @return Size of the stack
static inline bool gv_stack_has_capacity_(const gv_stack_t_ *stack) {
  return stack->next_slot != stack->limit;
}

/// Increase the stack capacity by a suitable amount, to allow inserting
/// at least one more item.
///
/// This function is a no-op if sufficient space is already available.
///
/// @param stack Stack to operate on
/// @param item_size Byte size of stack items
void gv_stack_increase_capacity_(gv_stack_t_ *stack, size_t item_size);

/// Return the index of a new slot to store an item being appended.
/// No item is yet assigned to that slot, so be careful...
///
/// @param stack Stack to operate on
/// @param item_size Byte size of stack items
/// @return Index of appended element.
static inline size_t gv_append_and_return_new_slot_(gv_stack_t_ *stack,
                                                    size_t item_size) {
  if ((stack)->next_slot == (stack)->limit) {
    gv_stack_increase_capacity_(stack, item_size);
  }
  size_t old_stack_size = gv_stack_size_(stack, item_size);
  void *old_next_slot = stack->next_slot;
  stack->next_slot = (void *)(((char *)old_next_slot) + item_size);
  ASAN_UNPOISON(old_next_slot, item_size);
  return old_stack_size;
}

/// Increase the stack capacity by a suitable amount, to allow inserting
/// at least one more item.
///
/// This function is a no-op if sufficient space is already available.
///
/// @param stack Stack to operate on
/// @param item_size Byte size of stack items
/// @return True if more memory could be allocated.
int gv_stack_try_increase_capacity_(gv_stack_t_ *stack, size_t item_size);

/// add an empty space for an item to the end of a stack
///
/// This function calls `exit` on failure.
///
/// You can think of this macro as having the C type:
///     void GV_STACK_APPEND_(gv_stack_t_ *stack, <type> item, size_t item_size)
///
/// @param stack Stack to operate on
/// @param item_size Byte size of each stack item
/// @return Index of the new (empty) slot
#define GV_STACK_APPEND_(stack, item, item_size)                               \
  do {                                                                         \
    if ((stack)->next_slot == (stack)->limit) {                                \
      gv_stack_increase_capacity_(stack, item_size);                           \
    }                                                                          \
    memcpy((stack)->next_slot, &(item), item_size);                            \
    stack->next_slot = (void *)(((char *)(stack)->next_slot) + item_size);     \
  } while (0)

/// try to append a new item to a stack, with size provided
///
/// You can think of this macro as having the C type:
///
///     bool GV_STACK_TRY_APPEND_(gv_stack_t_ *stack, <type> item,
///                               size_t item_size)
///
/// @param stack Stack to operate on
/// @param item Pointer to item to append
/// @param item_size Byte size of each stack item
/// @return True if the append succeeded
#define GV_STACK_TRY_APPEND_(stack, item, item_size)                           \
  ((((stack)->impl.next_slot !=                                                \
     (stack)->impl.limit) /* already has capacity */                           \
    || gv_stack_try_increase_capacity_((stack)->impl,                          \
                                       item_size)) /* allocation succeeds */   \
   &&                                                                          \
   (memcpy((stack)->impl.next_slot, &(item), item_size), /* copy item */       \
    ((stack)->impl.next_slot = (void *)(((char *)(stack)->impl.next_slot) +    \
                                        (item_size))), /* update next_slot */  \
    true)                                              /* succeeded */         \
  )

/// retrieve a pointer to the last item in a stack.
///
/// You can think of this macro as having one of the C types:
///
///   void *GV_STACK_BACK_(gv_stack_t_ *stack, size_t item_size)
///   const void *GV_STACK_BACK_(const gv_stack_t_ *stack, size_t item_size);
///
/// @param stack Stack to operate on
/// @param item_size Byte size of each stack item
/// @return Pointer to the last item in the stack
#define GV_STACK_BACK_(stack, item_size)                                       \
  ((void *)(((char *)(stack)->next_slot) - (item_size)))

/// retrieve the index of the last item in a stack
///
/// You can think of this macro as having one of the C types:
///
///   size_t GV_STACK_BACK_IDX_(gv_stack_t_ *stack, size_t item_size)
///   const size_t GV_STACK_BACK_(const gv_stack_t_ *stack, size_t item_size);
///
/// @param stack Stack to operate on
/// @param item_size Byte size of each stack item
/// @return Index of last item in the stack
#define GV_STACK_BACK_IDX_(stack, item_size)                                   \
  (((((char *)(stack)->next_slot) - (((char *)(stack)->base))) /               \
    (item_size)) -                                                             \
   1)

/// check that provided index is in stack bounds
///
/// bool GV_STACK_INDEX_IN_BOUNDS_(const GV_STACK(<type>) *stack, size_t index,
///                                size_t item_size);
///
/// @param stack Stack to operate on
/// @param index Index of item to lookup
/// @return Slot index corresponding to the given index
#define GV_STACK_INDEX_IN_BOUNDS_(stack, index, item_size)                     \
  (((stack)->impl.base != NULL) && ((stack)->impl.next_slot != NULL) &&        \
   (((char *)((stack)->impl.base) + (index) * (item_size)) <                   \
    ((char *)((stack)->impl.next_slot))))

/// run the destructor of a stack on a given slot
///
/// Though this uses the public type `STACK(<type>)` defined in stack.h, this is
/// an internal API not expected to be called by anything other than the macros
/// in stack.h.
///
/// You can think of this macro as having the following C type:
///
///   void GV_STACK_DTOR_(STACK(<type>) *stack, size_t slot);
///
/// @param stack Stack to operate on
/// @param slot Slot to destruct
#define GV_STACK_DTOR_(stack, slot)                                            \
  do {                                                                         \
    if ((stack)->dtor == GV_STACK_DTOR_FREE) {                                 \
      /* we need to juggle the element into a pointer to avoid compilation */  \
      /* errors from this (untaken) branch when the element type is not a  */  \
      /* pointer */                                                            \
      void *ptr_;                                                              \
      sizeof((stack)->base[0]) == sizeof(ptr_)                                 \
          ? (void)0                                                            \
          : (void)(fprintf(stderr, "stack element type is not a pointer, but " \
                                   "`free` used as destructor\n"),             \
                   abort());                                                   \
      memcpy(&ptr_, &(stack)->base[slot], sizeof(ptr_));                       \
      free(ptr_);                                                              \
    } else if ((stack)->dtor != NULL) {                                        \
      (stack)->dtor((stack)->base[slot]);                                      \
    }                                                                          \
  } while (0)

/// remove all items from a stack
///
/// @param stack Stack to operate on
/// @param item_size Byte size of stack items
UTIL_API void gv_stack_clear_(gv_stack_t_ *stack, size_t item_size);

/// reserve space for new items in a stack
///
/// This function is a no-op if sufficient space is already available.
///
/// @param stack Stack to operate on
/// @param capacity Total number of slots to make available
/// @param item_size Byte size of stack items
UTIL_API void gv_stack_reserve_(gv_stack_t_ *stack, size_t capacity,
                                size_t item_size);

/// sort a stack
///
/// @param stack Stack to operate on
/// @param cmp Comparator for ordering list items
/// @param item_size Byte size of each list item
UTIL_API void gv_stack_sort_(gv_stack_t_ *stack,
                             int (*cmp)(const void *, const void *),
                             size_t item_size);

/// reverse the item order of a stack
///
/// @param stack Stack to operate on
/// @param item_size Byte size of each stack item
UTIL_API void gv_stack_reverse_(gv_stack_t_ *stack, size_t item_size);

/// free resources associated with a stack
///
/// @param stack Stack to operate on
UTIL_API void gv_stack_free_(gv_stack_t_ *stack);

/// remove and return the last item of a stack
///
/// @param stack Stack to operate on
/// @param [out] into Destination to pop the item into
/// @param item_size Byte size of each stack item
UTIL_API void gv_stack_pop_back_(gv_stack_t_ *stack, void *into,
                                 size_t item_size);

/// just remove the last item of a stack
///
/// void GV_STACK_DROP_BACK_(gv_stack_t_ *stack, size_t item_size);
///
/// @param stack Stack to operate on
/// @param item_size Byte size of each stack item
#define GV_STACK_DROP_BACK_(stack, item_size)                                  \
  do {                                                                         \
    (stack)->next_slot = (void *)(((char *)(stack)->next_slot) - item_size);   \
    ASAN_POISON((stack)->next_slot, item_size);                                \
  } while (0)

#ifdef __cplusplus
}
#endif
