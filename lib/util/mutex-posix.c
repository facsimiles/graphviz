/// @file
/// @brief Implementation of mutex.h backed by POSIX threads

#include "config.h"

#include <assert.h>
#include <pthread.h>
#include <stdlib.h>
#include <util/mutex.h>

struct mutex {
  pthread_mutex_t impl;
};

mutex_t *gv_mutex_new(void) {

  pthread_mutexattr_t attr;
  mutex_t *m = NULL;
  mutex_t *rc = NULL;

  if (pthread_mutexattr_init(&attr) != 0) {
    return NULL;
  }

  if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK) != 0) {
    goto done;
  }

  m = calloc(1, sizeof(*m));
  if (m == NULL) {
    goto done;
  }

  if (pthread_mutex_init(&m->impl, &attr) != 0) {
    free(m);
    m = NULL;
    goto done;
  }

  // success
  rc = m;
  m = NULL;

done:
  if (m != NULL) {
    (void)pthread_mutex_destroy(&m->impl);
  }
  free(m);
  (void)pthread_mutexattr_destroy(&attr);

  return rc;
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
