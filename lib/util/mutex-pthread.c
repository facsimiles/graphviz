/// @file
/// @brief Implementation of mutex.h backed by POSIX threads

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <util/mutex.h>

struct mutex {
  pthread_mutex_t impl;
};

mutex_t *gv_mutex_new(void) {
  mutex_t *const m = calloc(1, sizeof(*m));
  if (m == NULL) {
    return NULL;
  }
  if (pthread_mutex_init(&m->impl, NULL) != 0) {
    free(m);
    return NULL;
  }
  return m;
}

void gv_mutex_lock(mutex_t *m) {
  assert(m != NULL);
  const int r = pthread_mutex_lock(&m->impl);
  assert(r == 0);
  (void)r;
}

void gv_mutex_unlock(mutex_t *m) {
  assert(m != NULL);
  const int r = pthread_mutex_unlock(&m->impl);
  assert(r == 0);
  (void)r;
}

void gv_mutex_free(mutex_t *m) {
  assert(m != NULL);
  (void)pthread_mutex_destroy(&m->impl);
  free(m);
}
