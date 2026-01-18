/// @file
/// @brief Unit tester for dword.c

#ifdef NDEBUG
#error "this is not intended to be compiled with assertions off"
#endif

#define NO_CONFIG // suppress include of config.h in dword.c

#include <util/dword.c>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/unused.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdatomic.h>
#include <util/prisize_t.h>

#ifdef USE_C11_THREADS
#include <threads.h>
#elif defined(USE_PTHREADS)
#include <pthread.h>
#endif

/// compare 2 dwords
static bool eq(dword_t a, dword_t b) {
  return memcmp(&a, &b, sizeof(a)) == 0;
}

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
  _Atomic dword_t * ptr;
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

static void test_load(ctxt_t *ctxt) {
  const dword_t val = gv_dword_atomic_load(ctxt->ptr);
  assert(eq(val, ctxt->init_val));
}

typedef struct {
  void (*test)(ctxt_t*);
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
    mt[i] = (mt_t){.test = test, .ctxt = (ctxt_t){.ptr = &reference, .init_val = val, .thread_id = i, .n_threads = n_threads}};
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
  for (size_t i =0; i < n_threads; ++i) {
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
    ctxt_t ctxt = {.ptr = &reference, .init_val = make()}; 
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

#define RUN(testcase, thread_count) \
do { \
  const size_t n_threads = (thread_count); \
    printf("running test_%s", #testcase);                                         \
    if (n_threads > 1) { \
      printf(" with %" PRISIZE_T " threads...", n_threads); \
    } else { \
    printf(" single threaded..."); \
    } \
    fflush(stdout);                                                            \
    if (!run(test_##testcase, n_threads)) { \
      printf("SKIPPED\n"); \
    } else { \
    printf("OK\n");                                                            \
    } \
  } while (0)

RUN(init, 1);
RUN(load, 1);
RUN(load, 2);
RUN(load, 10);

#undef RUN

  return EXIT_SUCCESS;
}
