/// @file
/// @brief Unit tester for dword.c

#ifdef NDEBUG
#error "this is not intended to be compiled with assertions off"
#endif

#define NO_CONFIG // suppress include of config.h in dword.c

#include <assert.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/dword.c>
#include <util/prisize_t.h>
#include <util/unused.h>

#ifdef USE_C11_THREADS
#include <threads.h>
#elif defined(USE_PTHREADS)
#include <pthread.h>
#endif

/// compare 2 dwords
static bool eq(dword_t a, dword_t b) { return memcmp(&a, &b, sizeof(a)) == 0; }

#if 0
static void set(_Atomic dword_t *dst, int8_t src) {
  dword_t s = dword_new();
  memcpy(&s, &src, sizeof(src));
  atomic_store_explicit(dst, s, memory_order_release);
}
#else
static dword_t make(void) {
  dword_t ret = gv_dword_new();

  uint8_t init[sizeof(dword_t)] = {0};
  for (uint8_t i = 0; i < sizeof(init); ++i) {
    init[i] = i + 1;
  }
  memcpy(&ret, &init, sizeof(ret));

  return ret;
}
#endif

typedef struct {
  _Atomic dword_t *ptr;
  dword_t init_val;
  size_t thread_id;
  size_t n_threads;
} ctxt_t;

/// initialization should always set the same value
static void test_init(UNUSED ctxt_t *ctxt) {
  const dword_t a = gv_dword_new();
  const dword_t b = gv_dword_new();
  assert(eq(a, b));
}

/// `gv_dword_atomic_load` should read the correct initial value
static void test_load(ctxt_t *ctxt) {
  const dword_t val = gv_dword_atomic_load(ctxt->ptr);
  assert(eq(val, ctxt->init_val));
}

/// `gv_dword_atomic_store` should write as expected
static void test_store(ctxt_t *ctxt) {

  // derive an arbitrary different value to the initial one
  dword_t new;
  {
    char *const d = (char *)&new;
    const char *const s = (char *)&ctxt->init_val;
    for (size_t i = 0; i < sizeof(dword_t); ++i) {
      d[i] = s[sizeof(dword_t) - i - 1];
    }
  }
  assert(!eq(ctxt->init_val, new));

  // overwrite the initial value with this
  gv_dword_atomic_store(ctxt->ptr, new);

  // read back the stored value
  const dword_t result = atomic_load_explicit(ctxt->ptr, memory_order_acquire);

  // This should be what we wrote, even under multi-threading as all threads are
  // trying to write the same value. That is, we should see no torn writes.
  assert(eq(result, new));
}

static void test_cas(ctxt_t *ctxt) {

  // Construct a `dword_t` that is our thread ID. Little endian vs big endian
  // does not matter here as we will only deal with one byte of the `dword_t`.
  assert(ctxt->thread_id <= UINT8_MAX && "thread ID will not fit in a byte");
  dword_t thread_id;
  memset(&thread_id, 0, sizeof(thread_id));
  memcpy(&thread_id, &(uint8_t){(uint8_t)ctxt->thread_id}, 1);

  // if we are the first thread, swap our ID over the initial value
  if (ctxt->thread_id == 0) {
    dword_t expected = ctxt->init_val;
    const bool r = gv_dword_atomic_cas(ctxt->ptr, &expected, thread_id);
    // this should succeed because no other thread’s CAS should be able to
    // proceed before us
    assert(r && "first CAS did not see initial value");
    assert(eq(expected, ctxt->init_val) && "succeeding CAS modified expected");
    return;
  }

  // try to swap in our ID, expecting the previous thread to write its ID first
  while (true) {
    dword_t previous;
    memset(&previous, 0, sizeof(previous));
    memcpy(&previous, &(uint8_t){(uint8_t)(ctxt->thread_id - 1)}, 1);
    if (gv_dword_atomic_cas(ctxt->ptr, &previous, thread_id)) {
      break;
    }
  }
}

static void test_cas_fail(ctxt_t *ctxt) {

  // derive an arbitrary different value to the initial one based on our thread
  // ID
  assert(ctxt->thread_id <= UINT8_MAX && "thread ID will not fit in a byte");
  dword_t new;
  memset(&new, 0, sizeof(new));
  memcpy(&new, &(uint8_t){(uint8_t)ctxt->thread_id}, 1);
  assert(!eq(ctxt->init_val, new));

  // derived another one as our expectation
  dword_t expect;
  memset(&expect, 0, sizeof(expect));
  memcpy(&expect, &(uint8_t){(uint8_t)ctxt->thread_id + 1}, 1);
  assert(!eq(ctxt->init_val, expect));
  assert(!eq(new, expect));

  // CAS should fail because we know it differs from the initial value
  const bool r = gv_dword_atomic_cas(ctxt->ptr, &expect, new);
  assert(!r && "CAS succeeded with mismatching expectation");

  // our expectation should have been updated with the old value
  assert(eq(expect, ctxt->init_val) && "failing CAS did not read old value");

  // the stored value should not have been modified
  const dword_t stored = atomic_load_explicit(ctxt->ptr, memory_order_acquire);
  assert(eq(stored, ctxt->init_val) && "failing CAS stored to destination");
}

static void test_xchg(ctxt_t *ctxt) {

  // derive an arbitrary different value to the initial one based on our thread
  // ID
  assert(ctxt->thread_id <= UINT8_MAX && "thread ID will not fit in a byte");
  dword_t new;
  memset(&new, 0, sizeof(new));
  memcpy(&new, &(uint8_t){(uint8_t)ctxt->thread_id}, 1);
  assert(!eq(ctxt->init_val, new));

  // exchange this with the current value
  const dword_t old = gv_dword_atomic_xchg(ctxt->ptr, new);

  // we should have seen either the initial value or something written by a
  // thread that is now ourselves
  bool ok = false;
  if (eq(old, ctxt->init_val)) {
    ok = true;
  } else {
    for (uint8_t i = 0; i < ctxt->n_threads; ++i) {
      if (i == ctxt->thread_id)
        continue;
      dword_t them;
      memset(&them, 0, sizeof(them));
      memcpy(&them, &i, 1);
      if (eq(old, them)) {
        ok = true;
        break;
      }
    }
  }
  assert(ok && "xchg returned unexpected value");
}

typedef struct {
  void (*test)(ctxt_t *);
  ctxt_t ctxt;
} mt_t;

#ifdef USE_C11_THREADS
#define TRAMPOLINE_RET_TYPE int
#else
#define TRAMPOLINE_RET_TYPE void *
#endif

static UNUSED TRAMPOLINE_RET_TYPE mt_trampoline(void *arg) {
  assert(arg != NULL);
  mt_t *s = arg;
  s->test(&s->ctxt);
  return 0;
}

static bool run_mt(void (*test)(ctxt_t *), size_t n_threads) {
  assert(test != NULL);
  assert(n_threads > 1);

  bool ret = true;

  // initialize reference location to an arbitrary value
  _Atomic dword_t reference;
  const dword_t val = make();
  atomic_store_explicit(&reference, val, memory_order_release);

  // setup a context per-thread, pointing at the same reference
  mt_t *const mt = calloc(n_threads, sizeof(mt[0]));
  assert(mt != NULL && "out of memory");
  for (size_t i = 0; i < n_threads; ++i) {
    mt[i] = (mt_t){.test = test,
                   .ctxt = (ctxt_t){.ptr = &reference,
                                    .init_val = val,
                                    .thread_id = i,
                                    .n_threads = n_threads}};
  }

#ifdef USE_C11_THREADS
  {
    thrd_t *const thread = calloc(n_threads, sizeof(thread[0]));
    assert(thread != NULL);

    // start the number of requested threads
    for (size_t i = 0; i < n_threads; ++i) {
      const int r = thrd_create(&thread[i], mt_trampoline, &mt[i]);
      assert(r == thrd_success);
    }

    // wait for them to finish
    for (size_t i = 0; i < n_threads; ++i) {
      int rc;
      const int r = thrd_join(thread[i], &rc);
      assert(r == thrd_success);
      assert(rc == 0);
    }

    free(thread);
  }
#elif defined(USE_PTHREADS)
  {
    pthread_t *const thread = calloc(n_threads, sizeof(thread[0]));
    assert(thread != NULL);

    // start the number of requested threads
    for (size_t i = 0; i < n_threads; ++i) {
      const int r = pthread_create(&thread[i], NULL, mt_trampoline, &mt[i]);
      assert(r == 0);
    }

    // wait for them to finish
    for (size_t i = 0; i < n_threads; ++i) {
      void *rc;
      const int r = pthread_join(thread[i], &rc);
      assert(r == 0);
      assert(rc == NULL);
    }

    free(thread);
  }
#else // multi-threading disabled
  // mark test case as skipped
  ret = false;
#endif

  free(mt);

  return ret;
}

static bool run_st(void (*test)(ctxt_t *)) {
  assert(test != NULL);

  _Atomic dword_t reference;
  /* initialize value to an arbitrary reference constant */
  ctxt_t ctxt = {.ptr = &reference, .init_val = make(), .n_threads = 1};
  atomic_store_explicit(&reference, ctxt.init_val, memory_order_release);
  test(&ctxt);

  return true;
}

static bool run(void (*test)(ctxt_t *), size_t n_threads) {
  assert(test != NULL);
  assert(n_threads > 0);

  if (n_threads == 1) {
    return run_st(test);
  }

  return run_mt(test, n_threads);
}

int main(void) {

#define RUN(testcase, thread_count)                                            \
  do {                                                                         \
    const size_t n_threads = (thread_count);                                   \
    printf("running test_%s", #testcase);                                      \
    if (n_threads > 1) {                                                       \
      printf(" with %" PRISIZE_T " threads...", n_threads);                    \
    } else {                                                                   \
      printf(" single threaded...");                                           \
    }                                                                          \
    fflush(stdout);                                                            \
    if (!run(test_##testcase, n_threads)) {                                    \
      printf("SKIPPED\n");                                                     \
    } else {                                                                   \
      printf("OK\n");                                                          \
    }                                                                          \
  } while (0)

  RUN(init, 1);
  RUN(load, 1);
  RUN(load, 2);
  RUN(load, 10);
  RUN(store, 1);
  RUN(store, 2);
  RUN(store, 10);
  RUN(cas, 1);
  RUN(cas, 2);
  RUN(cas, 10);
  RUN(cas_fail, 1);
  RUN(cas_fail, 2);
  RUN(cas_fail, 10);
  RUN(xchg, 1);
  RUN(xchg, 2);
  RUN(xchg, 10);

#undef RUN

  return EXIT_SUCCESS;
}
