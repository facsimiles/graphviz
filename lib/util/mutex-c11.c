/// @file
/// @brief Implementation of mutex.h backed by C11 threads

#include "config.h"

#include <assert.h>
#include <stdlib.h>
#include <threads.h>
#include <util/mutex.h>

struct mutex {
  mtx_t impl;
};

mutex_t *gv_mutex_new(void) {
  mutex_t *const m = calloc(1, sizeof(*m));
  if (m == NULL) {
    return NULL;
  }
  if (mtx_init(&m->impl, mtx_plain) != thrd_success) {
    free(m);
    return NULL;
  }
  return m;
}

void gv_mutex_lock(mutex_t *m) {
  assert(m != NULL);
  const int r = mtx_lock(&m->impl);
  assert(r == thrd_success);
  (void)r;
}

void gv_mutex_unlock(mutex_t *m) {
  assert(m != NULL);
  const int r = mtx_unlock(&m->impl);
  assert(r == thrd_success);
  (void)r;
}

void gv_mutex_free(mutex_t *m) {
  assert(m != NULL);
  mtx_destroy(&m->impl);
  free(m);
}
