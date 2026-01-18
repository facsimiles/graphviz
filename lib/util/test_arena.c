/// @file
/// @brief Basic unit tester for arena.c

#ifdef NDEBUG
#error this is not intended to be compiled with assertions off
#endif

#define NO_CONFIG // suppress include of config.h in arena.c

#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/arena.c>

/// trivial lifecycle of an arena
static void test_basic_lifecycle(void) {

  // create a new arena
  arena_t a = {0};

  // reset the arena
  gv_arena_reset(&a);
}

// a more realistic lifecycle usage of an arena
static void test_lifecycle(void) {

  // some arbitrary sizes we will allocate
  static const size_t allocations[] = {123,  1,    42,   2,    3,
                                       4096, 1024, 1023, 20000};

  // length of the above array
  enum { allocations_len = sizeof(allocations) / sizeof(allocations[0]) };

  // somewhere to store pointers to memory we allocate
  unsigned char *p[allocations_len];

  // create a new arena
  arena_t a = {0};

  for (size_t i = 0; i < allocations_len; ++i) {

    // create the allocation
    const size_t ALIGN = 8;
    p[i] = gv_arena_alloc(&a, ALIGN, allocations[i]);
    assert(p[i] != NULL);
    assert(((uintptr_t)p[i] & (ALIGN - 1)) == 0);

    // confirm we can write to it without faulting
    for (size_t j = 0; j < allocations[i]; ++j) {
      volatile unsigned char *q = (volatile unsigned char *)&p[i][j];
      *q = (unsigned char)(j % UCHAR_MAX);
    }

    // confirm we can read back what we wrote
    for (size_t j = 0; j < allocations[i]; ++j) {
      volatile unsigned char *q = (volatile unsigned char *)&p[i][j];
      assert(*q == (unsigned char)(j % UCHAR_MAX));
    }
  }

  // free a few allocations out of order from that in which we allocated
  gv_arena_free(&a, p[3], allocations[3]);
  gv_arena_free(&a, p[6], allocations[6]);
  gv_arena_free(&a, p[5], allocations[5]);

  // free the rest of the allocations in one sweep
  gv_arena_reset(&a);

  // try the allocate-write-read test in reverse order
  for (size_t i = allocations_len - 1;; --i) {

    // create the allocation
    const size_t ALIGN = 8;
    p[i] = gv_arena_alloc(&a, ALIGN, allocations[i]);
    assert(p[i] != NULL);
    assert(((uintptr_t)p[i] & (ALIGN - 1)) == 0);

    // confirm we can write to it without faulting
    for (size_t j = 0; j < allocations[i]; ++j) {
      volatile unsigned char *q = (volatile unsigned char *)&p[i][j];
      *q = (unsigned char)(j % UCHAR_MAX);
    }

    // confirm we can read back what we wrote
    for (size_t j = 0; j < allocations[i]; ++j) {
      volatile unsigned char *q = (volatile unsigned char *)&p[i][j];
      assert(*q == (unsigned char)(j % UCHAR_MAX));
    }

    if (i == 0) {
      break;
    }
  }

  // clean up
  gv_arena_reset(&a);
}

/// basic test of our strdup analogue
static void test_strdup(void) {

  // create a new arena
  arena_t a = {0};

  const char s[] = "hello world";

  // ask arena to strdup this
  char *const t = gv_arena_strdup(&a, s);
  assert(t != NULL);

  // we should have received back the duplicated content
  assert(strcmp(s, t) == 0);

  // now ask the arena to strdup something it produced itself
  char *const u = gv_arena_strdup(&a, t);
  assert(u != NULL);

  // we should still have the correct original string content
  assert(strcmp(s, u) == 0);

  // clean up
  gv_arena_reset(&a);
}

int main(void) {

#define RUN(t)                                                                 \
  do {                                                                         \
    printf("running test_%s... ", #t);                                         \
    fflush(stdout);                                                            \
    test_##t();                                                                \
    printf("OK\n");                                                            \
  } while (0)

  RUN(basic_lifecycle);
  RUN(lifecycle);
  RUN(strdup);

#undef RUN

  return EXIT_SUCCESS;
}
