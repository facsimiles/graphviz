/// @file
/// @brief type-generic dynamically expanding single-ended stack
/// @ingroup cgraph_utils
///
/// The code in this header is structured as a public API made up of macros that
/// do as little as possible before handing off to internal functions.
///
/// If you are familiar with the concept of a dynamically expanding array like
/// C++’s `std::vector`, the only things that will likely throw you off are:
///   1. The genericity of `STACK` is implemented through a `union` that
///      overlaps the `base` member of a `stack_t_` (a generic stack core)
///      with a typed pointer. This design involves the public macros dealing in
///      the `STACK(foo)` type while the private functions deal in `stack_t_`s.
///
/// Some general terminology you may see in function/macro names:
///   base – the start of heap-allocated array of items
///   dtor – destructor
///   item – a stack element
///   slot – an item-sized space in the stack, offset recorded from base
///
/// Unlike the more general list declared in list.h, we only append and remove
/// from the tail of the stack, so the first item is always at element 0 of the
/// array pointed to by base.
///
/// Some unorthodox idioms you may see used in this file:
///   • `(void)(foo == bar)` as a way to force the compiler to type-check that
///     `foo` and `bar` have compatible types. This is the best we can do for
///     pointer compatibility checks without `typeof`.
///   • `(void)(sizeof(foo) == sizeof(bar) ? (void)0 : (void)(…,abort())` as an
///     even weaker version of the above, for when we need to delay a check to
///     runtime instead of compile-time. This is a very unreliable check for
///     `foo` and `bar` being the same type, so should be avoided wherever
///     possible.

#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <util/gv_stack-private.h>

#ifdef __cplusplus
extern "C" {
#endif

static_assert(offsetof(gv_stack_t_, base) == 0,
              "GV_STACK(<type>).base and GV_STACK(<type>).impl.base "
              "will not alias each other");

/// stack data structure
///
/// Typical usage:
///
///   STACK(int) my_int_stack = {0};
#define GV_STACK(type)                                                         \
  struct {                                                                     \
    union {                                                                    \
      type *base;                                                              \
      gv_stack_t_ impl;                                                        \
    }; /**< backing storage */                                                 \
    void (*dtor)(type); /**< optional destructor */                            \
    type scratch;       /**< temporary space for storing off-list items */     \
  }

/// sentinel value to indicate you want `free` to be used as a stack destructor
///
/// Sample usage:
///
///   GV_STACK(char *) my_strings = {.dtor = GV_STACK_DTOR_FREE};
#define GV_STACK_DTOR_FREE ((void *)1)

/// get the number of elements in a stack
///
/// You can think of this macro as having the C type:
///
///   size_t GV_STACK_SIZE(const GV_STACK(<type>) *stack);
///
/// @param stack Stack to inspect
/// @return Size of the stack
#define GV_STACK_SIZE(stack)                                                   \
  gv_stack_size_(&(stack)->impl, sizeof((stack)->base[0]))

/// does this stack contain no elements?
///
/// You can think of this macro as having the C type:
///
///   bool GV_STACK_IS_EMPTY(const GV_STACK(<type>) *stack);
///
/// @param stack Stack to inspect
/// @return True if the stack is empty
#define GV_STACK_IS_EMPTY(stack) (gv_stack_is_empty_(&(stack)->impl))

/// Try to add a new item to the top/back/end of a stack,
/// without allocating more space.
///
/// You can think of this macro as having the C type:
///
///   bool GV_STACK_TRY_APPEND(GV_STACK(<type>) *stack, <type> item);
///
/// @param stack Stack to operate on
/// @param item Item to push
/// @return True if the push succeeded
#define GV_STACK_TRY_APPEND(stack, item)                                       \
  ((gv_stack_has_capacity_(&((stack)->impl))) &&                               \
   ((stack)->base[gv_append_and_return_new_slot_(                              \
        &(stack)->impl, sizeof((stack)->base[0]))] = (item),                   \
    true))

/// add an item to the top/end/back of a stack
///
/// You can think of this macro as having the C type:
///
///   void GV_STACK_APPEND(GV_STACK(<type>) *stack, <type> item);
///
/// This macro succeeds or exits on out-of-memory; it never return failure.
///
/// @param stack Stack to operate on
/// @param item Element to append
#define GV_STACK_APPEND(stack, item)                                           \
  do {                                                                         \
    if (gv_stack_has_capacity_(&((stack)->impl))) {                            \
      /* fast path */                                                          \
      size_t last_idx = gv_append_and_return_new_slot_(                        \
          &(stack)->impl, sizeof((stack)->base[0]));                           \
      (stack)->base[last_idx] = (item);                                        \
    } else {                                                                   \
      /* slow path -- getting a slot may change item, so copy it first */      \
      (stack)->scratch = (item);                                               \
      size_t last_idx = gv_append_and_return_new_slot_(                        \
          &(stack)->impl, sizeof((stack)->base[0]));                           \
      (stack)->base[last_idx] = (stack)->scratch;                              \
    }                                                                          \
  } while (0);

/// retrieve a pointer to the last item in a stack
///
/// You can think of this macro as having one of the C types:
///
///   <type> *GV_STACK_BACK(GV_STACK(<type>) *stack);
///   const <type> *GV_STACK_BACK(const GV_STACK(<type>) *stack);
///
/// @param stack Stack to operate on
/// @return Pointer to the last item in the stack
#define GV_STACK_BACK(stack)                                                   \
  (&(stack)->base[GV_STACK_BACK_IDX_(&(stack)->impl, sizeof((stack)->base[0]))])

/// update the value of an item in the stack
///
/// You can think of this macro as having the C type:
///
///   void GV_STACK_SET(GV_STACK(<type>) *stack, size_t index, <type> item);
///
/// @param stack Stack to operate on
/// @param index Index of item to update
/// @param item New value to set
#define GV_STACK_SET(stack, index, item)                                       \
  do {                                                                         \
    const size_t slot_ = (size_t)(index);                                      \
    assert(                                                                    \
        GV_STACK_INDEX_IN_BOUNDS_((stack), slot_, sizeof((stack)->base[0])));  \
    GV_STACK_DTOR_((stack), slot_);                                            \
    (stack)->base[slot_] = (item);                                             \
  } while (0)

/// retrieve an item from a stack
///
/// You can think of this macro as having the C type:
///
///   <type> GV_STACK_GET(const GV_STACK(<type>) *stack, size_t index);
///
/// @param stack Stack to operate on
/// @param index Item index to get
/// @return Item at the given index
#define GV_STACK_GET(stack, index)                                             \
  (assert(                                                                     \
       GV_STACK_INDEX_IN_BOUNDS_((stack), index, sizeof((stack)->base[0]))),   \
   (stack)->base[(index)])

/// remove all items from a stack
///
/// You can think of this macro as having the C type:
///
///   void GV_STACK_CLEAR(GV_STACK(<type>) *stack);
///
/// @param stack Stack to clear
#define GV_STACK_CLEAR(stack)                                                  \
  do {                                                                         \
    for (size_t i_ = 0; i_ < GV_STACK_SIZE(stack); ++i_) {                     \
      const size_t slot_ = i_;                                                 \
      GV_STACK_DTOR_((stack), slot_);                                          \
    }                                                                          \
    gv_stack_clear_(&(stack)->impl, sizeof((stack)->base[0]));                 \
  } while (0)

/// reserve space for new items in a stack
///
/// You can think of this macro as having the C type:
///
///   void GV_STACK_RESERVE(GV_STACK(<type>) *stack, size_t capacity);
///
/// @param stack Stack to operate on
/// @param capacity Total number of item slots to make available
#define GV_STACK_RESERVE(stack, capacity)                                      \
  gv_stack_reserve_(&(stack)->impl, capacity, sizeof((stack)->base[0]))

/// sort a stack
///
/// You can think of this macro as having the C type:
///
///   void GV_STACK_SORT(GV_STACK(<type>) *stack,
///                      int (*cmp)(const void *a, const void *b));
///
/// @param stack Stack to operate on
/// @param cmp How to compare two stack items
#define GV_STACK_SORT(stack, cmp)                                              \
  gv_stack_sort_(&(stack)->impl, (cmp), sizeof((stack)->base[0]))

/// reverse the item order of a stack
///
/// You can think of this macro as having the C type:
///
///   void GV_STACK_REVERSE(GV_STACK(<type>) *stack);
///
/// @param stack Stack to operate on
#define GV_STACK_REVERSE(stack)                                                \
  gv_stack_reverse_(&(stack)->impl, sizeof((stack)->base[0]))

/// free resources associated with a stack
///
/// You can think of this macro as having the C type:
///
///   void GV_STACK_FREE(GV_STACK(<type>) *stack);
///
/// After a call to this function, the stack is empty and may be reused.
///
/// @param stack Stack to free
#define GV_STACK_FREE(stack)                                                   \
  do {                                                                         \
    GV_STACK_CLEAR(stack);                                                     \
    gv_stack_free_(&(stack)->impl);                                            \
  } while (0)

/// Push an item on the top/end/back of the stack.
/// (Alias for `GV_STACK_APPEND`)
///
/// You can think of this macro as having the C type:
///
///   void GV_STACK_PUSH_BACK(GV_STACK(<type>) *stack, <type> item);
///
/// @param stack Stack to operate on
/// @param item Item to append
#define GV_STACK_PUSH_BACK(stack, item) GV_STACK_APPEND((stack), (item))

/// Remove and return the top/last item pushed on a stack.
///
/// You can think of this macro as having the C type:
///
///   <type> GV_STACK_POP_BACK(GV_STACK(<type>) *stack);
///
/// @param stack Stack to operate on
/// @return Popped item
#define GV_STACK_POP_BACK(stack)                                               \
  (gv_stack_pop_back_(&(stack)->impl, &(stack)->scratch,                       \
                      sizeof((stack)->base[0])),                               \
   (stack)->scratch)

/// Remove the top/back/last item of a stack.
///
/// You can think of this macro as having the C type:
///
///   void GV_STACK_DROP_BACK(GV_STACK(<type>) *stack);
///
/// This can be used to pop the last element when the caller does not need the
/// popped item.
///
/// @param stack Stack to operate on
#define GV_STACK_DROP_BACK(stack)                                              \
  GV_STACK_DROP_BACK_(&(stack)->impl, sizeof((stack)->base[0]))

#ifdef __cplusplus
}
#endif
