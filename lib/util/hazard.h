/// @file
/// @brief Hazard pointer API
///
/// “Hazard pointers”¹ are a technique for safe concurrent access and
/// reclamation of dynamically allocated data structure nodes. Given an upper
/// bound on the number of shared pointers that will be concurrently accessed,
/// they allow (1) protecting one thread’s pointer dereferences conflicting with
/// another thread’s freeing of the pointer and (2) safely postponing the
/// freeing of a pointer until all threads have finished using it.
///
/// The above is a high level description, but how and why hazard pointers work
/// is a deeper topic. The following papers are useful in understanding the
/// concept more fully:
///   1. Maged M. Michael, “Hazard Pointers: Safe Memory Reclamation for
///      Lock-Free Objects” in TPDS 15(8) 2004.
///   2. Maged M. Michael, “Safe Memory Reclamation for Dynamic Lock-Free
///      Objects Using Atomic Reads and Writes” in PODC 2002.
///   3. Maurice Herlihy, Victor Luchangco, and Mark Moir, “The Repeat Offender
///      Problem: A Mechanism for Supporting Dynamic-sized Lock-free Data
///      Structures” in LNCS vol 2508 2002.
/// (2) appears to be Michael’s first public articulation of the idea, but (1)
/// is a more thorough and clear explanation. (3) offers a third-party view on
/// this area of concurrency mechanisms and helps put hazard pointers in
/// perspective.
///
/// ¹ https://en.wikipedia.org/wiki/Hazard_pointer

#pragma once

#include <stddef.h>
#include <util/api.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct hazard_set hazard_set_t;

/// create a new hazard set
///
/// The `freefn` parameter can be `NULL` to indicate `free` should be used for
/// reclamation.
///
/// @param size Maximum number of pointers that will need to be concurrently
///   hazarded
/// @param freefn Function to reclaim pointers
/// @return A created hazard set on success or `NULL` on failure
UTIL_API hazard_set_t *gv_hazard_set_new(size_t size, void (*freefn)(void *));

/// mark a pointer as hazarded (protected for upcoming dereference)
///
/// It is assumed the caller holds no other hazarded pointer. That is, only one
/// pointer per index can be hazarded concurrently.
///
/// @param set Hazard set used for coordination
/// @param index Hazard set index the caller owns
/// @param target The pointer to hazard
UTIL_API void gv_hazard(hazard_set_t *set, size_t index, void *target);

/// remove hazarding protection from a pointer (done with dereferencing)
///
/// @param set Hazard set used for coordination
/// @param index Hazard set index previously protecting the target pointer
/// @param target Protected pointer to unprotect
UTIL_API void gv_unhazard(hazard_set_t *set, size_t index, void *target);

/// schedule freeing of a shared pointer
///
/// It is assumed that (1) the caller holds no hazarded pointer (their hazard
/// set index slot is empty) and (2) the caller has coordinated with other
/// threads such that no one else can acquire a copy of `target` _after_
/// execution of this function has begun. It is fine if other threads have a
/// hazarded copy of `target` already when this function is entered.
///
/// The `target` pointer is either freed immediately if no one else has it
/// hazarded or else is saved to be freed in future. All remaining unfreed saved
/// pointers are freed in `gv_hazard_set_free` if not before. The free function
/// that was passed in `set`’s construction in `gv_hazard_set_new` will be used
/// to free `target`.
///
/// @param set Hazard set used for coordination
/// @param index Hazard set index the caller owns
/// @param target Pointer to free
UTIL_API void gv_reclaim(hazard_set_t *set, size_t index, void *target);

/// destroy a hazard set
///
/// This function is not thread-safe. It is assumed that the caller is either
/// single-threaded or has coordinated with other threads such that they are no
/// longer using the set.
///
/// Passing `NULL` to this function is a no-op.
///
/// @param set Hazard set to deallocate
UTIL_API void gv_hazard_set_free(hazard_set_t *set);

#ifdef __cplusplus
}
#endif
