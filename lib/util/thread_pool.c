#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>
#include <util/thread_pool.h>

typedef enum { INIT = 0, PRIMED, RUNNING } phase_t;

typedef struct {
  thrd_t handle;
  bool created;
  size_t thread_id;
  mtx_t *starter;
  bool *ok;
  int (*entry)(size_t thread_id, void *state);
  void *original_state;
} thread_state_t;

static int trampoline(void *state) {
  thread_state_t *const s = state;

  if (mtx_lock(s->starter) != thrd_success) {
    return -1;
  }
  const bool ok = *s->ok;
  (void)mtx_unlock(s->starter);
  if (!ok) {
    return -1;
  }

  return s->entry(s->thread_id, s->original_state);
}

struct thread_pool {
  phase_t phase;
  thread_state_t *thread;
  size_t n_thread;
  mtx_t starter;
  bool starter_created;
  bool ok;
};

thread_pool_t *gv_thread_pool_new(size_t threads,
                                  int (*entry)(size_t thread_id, void *state),
                                  void *state) {

  thread_pool_t *const pool = malloc(sizeof(*pool));
  if (pool == NULL) {
    goto fail;
  }

  if (mtx_init(&pool->starter, mtx_plain) != thrd_success) {
    goto fail;
  }
  pool->starter_created = true;

  pool->thread = calloc(threads, sizeof(pool->thread[0]));
  if (threads > 0 && pool->thread == NULL) {
    goto fail;
  }
  pool->n_thread = threads;

  if (mtx_lock(&pool->starter) != thrd_success) {
    goto fail;
  }

  pool->phase = PRIMED;
  for (size_t i = 0; i < threads; ++i) {
    pool->thread[i].thread_id = i;
    pool->thread[i].starter = &pool->starter;
    pool->thread[i].ok = &pool->ok;
    pool->thread[i].entry = entry;
    pool->thread[i].original_state = state;
    if (thrd_create(&pool->thread[i].handle, trampoline, &pool->thread[i]) !=
        thrd_success) {
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
  assert(pool->phase == PRIMED);

  pool->phase = RUNNING;

  // release the threads
  pool->ok = true;
  (void)mtx_unlock(&pool->starter);
}

int gv_thread_pool_join(thread_pool_t *pool) {
  assert(pool != NULL);

  // if the threads are still blocked on the initial barrier, release them
  if (pool->phase == PRIMED) {
    pool->ok = false;
    (void)mtx_unlock(&pool->starter);
  }

  int res = 0;

  for (size_t i = pool->n_thread - 1; i != SIZE_MAX; --i) {
    if (!pool->thread[i].created) {
      continue;
    }
    int one_res;
    const int rc = thrd_join(pool->thread[i].handle, &one_res);
    if (rc != thrd_success) {
      return rc;
    }
    res |= one_res;
  }

  return res;
}

void gv_thread_pool_free(thread_pool_t *pool) {
  if (pool != NULL) {
    (void)gv_thread_pool_join(pool);
    free(pool->thread);
    if (pool->starter_created) {
      mtx_destroy(&pool->starter);
    }
  }
  free(pool);
}
