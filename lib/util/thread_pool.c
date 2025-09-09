#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <util/thread_pool.h>

typedef struct {
  pthread_t handle;
  bool created;
  size_t thread_id;
  pthread_mutex_t *starter;
  bool *ok;
  int (*entry)(size_t thread_id, void *state);
  void *original_state;
} thread_state_t;

static void *trampoline(void *state) {
  thread_state_t *const s = state;

  if (pthread_mutex_lock(s->starter) != 0) {
    return (void *)(intptr_t)-1;
  }
  const bool ok = *s->ok;
  (void)pthread_mutex_unlock(s->starter);
  if (!ok) {
    return (void *)(intptr_t)-1;
  }

  return (void *)(intptr_t)s->entry(s->thread_id, s->original_state);
}

struct thread_pool {
  thread_state_t *thread;
  size_t n_thread;
  pthread_mutex_t starter;
  bool starter_created;
  bool starter_held;
  bool ok;
};

thread_pool_t *gv_thread_pool_new(size_t threads,
                                  int (*entry)(size_t thread_id, void *state),
                                  void *state) {

  thread_pool_t *const pool = malloc(sizeof(*pool));
  if (pool == NULL) {
    goto fail;
  }

  if (pthread_mutex_init(&pool->starter, NULL) != 0) {
    goto fail;
  }
  pool->starter_created = true;

  pool->thread = calloc(threads, sizeof(pool->thread[0]));
  if (threads > 0 && pool->thread == NULL) {
    goto fail;
  }
  pool->n_thread = threads;

  if (pthread_mutex_lock(&pool->starter) != 0) {
    goto fail;
  }
  pool->starter_held = true;

  for (size_t i = 0; i < threads; ++i) {
    pool->thread[i].thread_id = i;
    pool->thread[i].starter = &pool->starter;
    pool->thread[i].ok = &pool->ok;
    pool->thread[i].entry = entry;
    pool->thread[i].original_state = state;
    if (pthread_create(&pool->thread[i].handle, NULL, trampoline,
                       &pool->thread[i]) != 0) {
      goto fail;
    }
    pool->thread[i].created = true;
  }

  return pool;

fail:
  gv_thread_pool_free(pool);
  return NULL;
}

void gv_thread_pool_start(thread_pool_t *pool) {
  assert(pool != NULL);
  assert(pool->starter_held);

  // release the threads
  pool->ok = true;
  (void)pthread_mutex_unlock(&pool->starter);
  pool->starter_held = false;
}

int gv_thread_pool_join(thread_pool_t *pool) {
  assert(pool != NULL);

  // if the threads are still blocked on the initial barrier, release them
  if (pool->starter_held) {
    pool->ok = false;
    (void)pthread_mutex_unlock(&pool->starter);
    pool->starter_held = false;
  }

  int res = 0;

  for (size_t i = pool->n_thread - 1; i != SIZE_MAX; --i) {
    if (!pool->thread[i].created) {
      continue;
    }
    void *one_res;
    const int rc = pthread_join(pool->thread[i].handle, &one_res);
    if (rc != 0) {
      return rc;
    }
    res |= (int)(intptr_t)one_res;
  }

  return res;
}

void gv_thread_pool_free(thread_pool_t *pool) {
  if (pool != NULL) {
    (void)gv_thread_pool_join(pool);
    free(pool->thread);
    if (pool->starter_created) {
      pthread_mutex_destroy(&pool->starter);
    }
  }
  free(pool);
}
