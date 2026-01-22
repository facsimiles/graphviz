/// @file
/// @brief Platform abstraction for mutex locks

#pragma once

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/api.h>
#include <util/exit.h>

/// opaque type of a mutex
typedef struct mutex mutex_t;

#ifdef __cplusplus
extern "C" {
#endif

/// create a new mutex
///
/// @return Created mutex on success or `NULL` on failure
UTIL_API mutex_t *gv_mutex_new(void);

/// acquire a mutex
UTIL_API void gv_mutex_lock(mutex_t *m);

/// release a mutex
///
/// Behavior is undefined if the caller does not hold the given mutex.
UTIL_API void gv_mutex_unlock(mutex_t *m);

/// deallocate resources associated with a mutex
UTIL_API void gv_mutex_free(mutex_t *m);

#ifdef __cplusplus
}
#endif

#ifndef __cplusplus

// `_Atomic` is a keyword in C, but not available at all in C++ <C++23. So we
// can only expose these functions to C includers.

#include <stdatomic.h>

/// `gv_mutex_new` for multi-threaded situations
///
/// Sometimes there is no safe known-single-threaded context in which to
/// initialize a mutex. POSIX threads has `PTHREAD_MUTEX_INITIALIZER` for this,
/// but there is not a good platform-independent answer to this problem. This
/// function offers something that can be safely called repeatedly from a
/// possibly multi-threaded context to initialize a pointer to a mutex. This
/// should be avoided if it is possible to call `gv_mutex_new` instead.
///
/// @param mptr [in,out] Mutex pointer to initialize
/// @return 0 on success
static inline int mutex_lazy_init(mutex_t *_Atomic *mptr) {

  // is the mutex already initialized?
  mutex_t *m = atomic_load_explicit(mptr, memory_order_acquire);
retry:
  if (m != NULL) {
    return 0;
  }

  // create a new mutex
  mutex_t *const attempt = gv_mutex_new();
  if (attempt == NULL) {
    return -1;
  }

  // make this globally visible
  if (!atomic_compare_exchange_strong_explicit(
          mptr, &m, attempt, memory_order_acq_rel, memory_order_acquire)) {
    // failed because we raced with another thread; retry
    gv_mutex_free(attempt);
    goto retry;
  }

  return 0;
}

/// `mutex_lazy_init`, exiting on failure
static inline void mutex_lazy_init_or_die(mutex_t *_Atomic *mptr) {
  assert(mptr != NULL);
  const int r = mutex_lazy_init(mptr);
  if (r != 0) {
    fputs("mutex_lazy_init failed\n", stderr);
    graphviz_exit(EXIT_FAILURE);
  }
}

#endif
