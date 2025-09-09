#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <threads.h>
#include <util/thread_pool.h>

typedef struct {
  size_t thread_id;
  mtx_t *starter;
  bool *ok;
  int (*entry)(size_t thread_id, void *state);
  void *original_state;
} trampoline_state_t;

static int trampoline(void *state) {
  trampoline_state_t *const s = state;

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
  thrd_t *thread;
  mtx_t starter;
  bool starter_created;
  bool ok;
  trampoline_state_t *state;
  size_t n_thread;
  size_t n_created;
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

  pool->state = calloc(threads, sizeof(pool->state[0]));
  if (threads > 0 && pool->state == NULL) {
    goto fail;
  }

  if (mtx_lock(&pool->starter) != thrd_success) {
    goto fail;
  }

  pool->n_thread = threads;
  for (size_t i = 0; i < threads; ++i) {
    pool->state[i] = (trampoline_state_t){.thread_id = i,
                                          .starter = &pool->starter,
                                          .ok = &pool->ok,
                                          .entry = entry,
                                          .original_state = state};
    if (thrd_create(&pool->thread[i], trampoline, &pool->state[i]) !=
        thrd_success) {
      (void)mtx_unlock(&pool->starter);
      goto fail;
    }
    ++pool->n_created;
  }

  // release the threads
  pool->ok = true;
  (void)mtx_unlock(&pool->starter);

  return pool;

fail:
  gv_thread_pool_free(pool);
  return NULL;
}

int gv_thread_pool_join(thread_pool_t *pool) {
  assert(pool != NULL);

  int res = 0;

  for (size_t i = pool->n_created - 1; i != SIZE_MAX; --i) {
    int one_res;
    const int rc = thrd_join(pool->thread[i], &one_res);
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
    free(pool->state);
    free(pool->thread);
    if (pool->starter_created) {
      mtx_destroy(&pool->starter);
    }
  }
  free(pool);
}
