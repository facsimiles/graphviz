/// @file
/// @brief basic unit tester for gv_stack.h

#ifdef NDEBUG
#error "this is not intended to be compiled with assertions off"
#endif

#define NO_CONFIG // suppress include of config.h in gv_stack.c

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/gv_stack.c>
#include <util/gv_stack.h>
#include <util/unused.h>

// test construction and destruction, with nothing in-between
static void test_create_reset(void) {
  GV_STACK(int) i = {0};
  GV_STACK_FREE(&i);
}

// a stack should start in a known initial state
static void test_init(void) {
  GV_STACK(int) i = {0};
  assert(GV_STACK_IS_EMPTY(&i));
  assert(GV_STACK_SIZE(&i) == 0);
}

// reset of an initialized stack should be OK and idempotent
static void test_init_reset(void) {
  GV_STACK(int) i = {0};
  GV_STACK_FREE(&i);
  GV_STACK_FREE(&i);
  GV_STACK_FREE(&i);
}

// repeated append
static void test_append(void) {
  GV_STACK(int) xs = {0};
  assert(GV_STACK_IS_EMPTY(&xs));

  for (size_t i = 0; i < 10; ++i) {
    GV_STACK_APPEND(&xs, (int)i);
    assert(GV_STACK_SIZE(&xs) == i + 1);
  }

  GV_STACK_FREE(&xs);
}

/// append should not be affected by surprising parameter expansion order
/// https://gitlab.com/graphviz/graphviz/-/issues/2734
static void test_2734_append(void) {
  GV_STACK(int) xs = {0};

  GV_STACK_APPEND(&xs, 42);
  GV_STACK_APPEND(&xs, GV_STACK_GET(&xs, GV_STACK_SIZE(&xs) - 1));
  assert(GV_STACK_GET(&xs, 1) == 42);

  GV_STACK_FREE(&xs);
}

/// try-append should not be affected by surprising parameter expansion order
/// https://gitlab.com/graphviz/graphviz/-/issues/2734
static void test_2734_try_append(void) {
  GV_STACK(int) xs = {0};

  GV_STACK_APPEND(&xs, 42);

  GV_STACK_APPEND(&xs, GV_STACK_GET(&xs, GV_STACK_SIZE(&xs) - 1));

  assert(GV_STACK_GET(&xs, 1) == 42);

  GV_STACK_FREE(&xs);
}

/// interleaved append and prepend
static void test_multiple_append(void) {
  GV_STACK(int) xs = {0};

  for (size_t i = 0; i < 10; ++i) {
    GV_STACK_APPEND(&xs, (int)i);
  }

  assert(GV_STACK_SIZE(&xs) == 10);
  assert(GV_STACK_GET(&xs, 0) == 0);
  assert(GV_STACK_GET(&xs, 1) == 1);
  assert(GV_STACK_GET(&xs, 2) == 2);
  assert(GV_STACK_GET(&xs, 3) == 3);
  assert(GV_STACK_GET(&xs, 4) == 4);
  assert(GV_STACK_GET(&xs, 5) == 5);
  assert(GV_STACK_GET(&xs, 6) == 6);
  assert(GV_STACK_GET(&xs, 7) == 7);
  assert(GV_STACK_GET(&xs, 8) == 8);
  assert(GV_STACK_GET(&xs, 9) == 9);

  GV_STACK_FREE(&xs);
}

static void test_get(void) {
  GV_STACK(int) xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    GV_STACK_APPEND(&xs, (int)i);
  }

  for (size_t i = 0; i < 10; ++i) {
    assert(GV_STACK_GET(&xs, i) == (int)i);
  }
  for (size_t i = 9;; --i) {
    assert(GV_STACK_GET(&xs, i) == (int)i);
    if (i == 0) {
      break;
    }
  }

  GV_STACK_FREE(&xs);
}

static void test_set(void) {
  GV_STACK(int) xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    GV_STACK_APPEND(&xs, (int)i);
  }

  for (size_t i = 0; i < 10; ++i) {
    GV_STACK_SET(&xs, i, (int)(i + 1));
    assert(GV_STACK_GET(&xs, i) == (int)i + 1);
  }
  for (size_t i = 9;; --i) {
    GV_STACK_SET(&xs, i, (int)i - 1);
    assert(GV_STACK_GET(&xs, i) == (int)i - 1);
    if (i == 0) {
      break;
    }
  }

  GV_STACK_FREE(&xs);
}

static void test_clear_empty(void) {
  GV_STACK(int) xs = {0};
  GV_STACK_CLEAR(&xs);
  assert(GV_STACK_IS_EMPTY(&xs));

  GV_STACK_FREE(&xs);
}

static void test_clear(void) {
  GV_STACK(int) xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    GV_STACK_APPEND(&xs, (int)i);
  }

  assert(!GV_STACK_IS_EMPTY(&xs));
  GV_STACK_CLEAR(&xs);
  assert(GV_STACK_IS_EMPTY(&xs));

  GV_STACK_FREE(&xs);
}

static void test_reserve(void) {
  GV_STACK(int) xs = {0};
  assert(!GV_STACK_TRY_APPEND(&xs, 1));
  GV_STACK_RESERVE(&xs, 0);
  assert(!GV_STACK_TRY_APPEND(&xs, 1));

  GV_STACK_RESERVE(&xs, 1);
  assert(GV_STACK_TRY_APPEND(&xs, 1));
  assert(!GV_STACK_TRY_APPEND(&xs, 2));

  GV_STACK_RESERVE(&xs, 5);
  GV_STACK_APPEND(&xs, 2);
  GV_STACK_APPEND(&xs, 3);
  GV_STACK_APPEND(&xs, 4);
  GV_STACK_APPEND(&xs, 5);
  assert(GV_STACK_POP_BACK(&xs) == 5);

  assert(GV_STACK_TRY_APPEND(&xs, 5));
  assert(!GV_STACK_TRY_APPEND(&xs, 6));

  GV_STACK_FREE(&xs);
}

// basic push then pop
static void test_push_one(void) {
  GV_STACK(int) s = {0};
  int arbitrary = 42;
  GV_STACK_PUSH_BACK(&s, arbitrary);
  assert(GV_STACK_SIZE(&s) == 1);
  int top = GV_STACK_POP_BACK(&s);
  assert(top == arbitrary);
  assert(GV_STACK_IS_EMPTY(&s));
  GV_STACK_FREE(&s);
}

static void push_then_pop(int count) {
  GV_STACK(int) s = {0};
  for (int i = 0; i < count; ++i) {
    GV_STACK_PUSH_BACK(&s, i);
    assert(GV_STACK_SIZE(&s) == (size_t)i + 1);
  }
  for (int i = count - 1;; --i) {
    assert(GV_STACK_SIZE(&s) == (size_t)i + 1);
    int p = GV_STACK_POP_BACK(&s);
    assert(p == i);
    if (i == 0) {
      break;
    }
  }
  GV_STACK_FREE(&s);
}

// push a series of items
static void test_push_then_pop_ten(void) { push_then_pop(10); }

// push enough to cause an expansion
static void test_push_then_pop_many(void) { push_then_pop(4096); }

// interleave some push and pop operations
static void test_push_pop_interleaved(void) {
  GV_STACK(int) s = {0};
  size_t size = 0;
  for (int i = 0; i < 4096; ++i) {
    if (i % 3 == 1) {
      int p = GV_STACK_POP_BACK(&s);
      assert(p == i - 1);
      --size;
    } else {
      GV_STACK_PUSH_BACK(&s, i);
      ++size;
    }
    assert(GV_STACK_SIZE(&s) == size);
  }
  GV_STACK_FREE(&s);
}

/// an int comparer
static int cmp_int(const void *x, const void *y) {
  const int *a = x;
  const int *b = y;
  if (*a < *b) {
    return -1;
  }
  if (*a > *b) {
    return 1;
  }
  return 0;
}

/// sort on an empty stack should be a no-op
static void test_sort_empty(void) {
  GV_STACK(int) xs = {0};
  GV_STACK_SORT(&xs, cmp_int);
  assert(GV_STACK_SIZE(&xs) == 0);
  GV_STACK_FREE(&xs);
}

static void test_sort(void) {
  GV_STACK(int) xs = {0};

  // a stack of ints in an arbitrary order
  const int ys[] = {4, 2, 10, 5, -42, 3};

  // setup this stack and sort it
  for (size_t i = 0; i < sizeof(ys) / sizeof(ys[0]); ++i) {
    GV_STACK_APPEND(&xs, ys[i]);
  }
  GV_STACK_SORT(&xs, cmp_int);

  // we should now have a sorted version of `ys`
  assert(GV_STACK_SIZE(&xs) == sizeof(ys) / sizeof(ys[0]));
  assert(GV_STACK_GET(&xs, 0) == -42);
  assert(GV_STACK_GET(&xs, 1) == 2);
  assert(GV_STACK_GET(&xs, 2) == 3);
  assert(GV_STACK_GET(&xs, 3) == 4);
  assert(GV_STACK_GET(&xs, 4) == 5);
  assert(GV_STACK_GET(&xs, 5) == 10);

  GV_STACK_FREE(&xs);
}

/// sorting an already sorted stack should be a no-op
static void test_sort_sorted(void) {
  GV_STACK(int) xs = {0};
  const int ys[] = {-42, 2, 3, 4, 5, 10};

  for (size_t i = 0; i < sizeof(ys) / sizeof(ys[0]); ++i) {
    GV_STACK_APPEND(&xs, ys[i]);
  }
  GV_STACK_SORT(&xs, cmp_int);

  for (size_t i = 0; i < sizeof(ys) / sizeof(ys[0]); ++i) {
    assert(GV_STACK_GET(&xs, i) == ys[i]);
  }

  GV_STACK_FREE(&xs);
}

typedef struct {
  int x;
  int y;
} pair_t;

/// a pair comparer, using only the first element
static int cmp_pair(const void *x, const void *y) {
  const pair_t *a = x;
  const pair_t *b = y;
  if (a->x < b->x) {
    return -1;
  }
  if (a->x > b->x) {
    return 1;
  }
  return 0;
}

/// sorting a complex type should move entire values of the type together
static void test_sort_complex(void) {
  GV_STACK(pair_t) xs = {0};

  const pair_t ys[] = {{1, 2}, {-2, 3}, {-10, 4}, {0, 7}};

  for (size_t i = 0; i < sizeof(ys) / sizeof(ys[0]); ++i) {
    GV_STACK_APPEND(&xs, ys[i]);
  }
  GV_STACK_SORT(&xs, cmp_pair);

  assert(GV_STACK_SIZE(&xs) == sizeof(ys) / sizeof(ys[0]));
  assert(GV_STACK_GET(&xs, 0).x == -10);
  assert(GV_STACK_GET(&xs, 0).y == 4);
  assert(GV_STACK_GET(&xs, 1).x == -2);
  assert(GV_STACK_GET(&xs, 1).y == 3);
  assert(GV_STACK_GET(&xs, 2).x == 0);
  assert(GV_STACK_GET(&xs, 2).y == 7);
  assert(GV_STACK_GET(&xs, 3).x == 1);
  assert(GV_STACK_GET(&xs, 3).y == 2);

  GV_STACK_FREE(&xs);
}

/// reverse on an empty stack should be a no-op
static void test_reverse_empty(void) {
  GV_STACK(int) xs = {0};
  GV_STACK_REVERSE(&xs);
  assert(GV_STACK_SIZE(&xs) == 0);
  GV_STACK_FREE(&xs);
}

static void test_reverse(void) {
  GV_STACK(int) xs = {0};

  // a stack of ints in an arbitrary order
  const int ys[] = {4, 2, 10, 5, -42, 3};
  size_t n = sizeof(ys) / sizeof(ys[0]);
  for (size_t l = 0; l < n; ++l) {
    GV_STACK_CLEAR(&xs);

    // setup this stack and reverse it
    for (size_t i = 0; i < l; ++i) {
      GV_STACK_APPEND(&xs, ys[i]);
    }
    GV_STACK_REVERSE(&xs);

    // contents should be reversed
    assert(GV_STACK_SIZE(&xs) == l);
    for (size_t i = 0; i < l; ++i) {
      assert(GV_STACK_GET(&xs, i) == ys[l - 1 - i]);
    }
  }
  GV_STACK_FREE(&xs);
}

/// reversing a complex type should move entire values together
static void test_reverse_complex(void) {
  GV_STACK(pair_t) xs = {0};

  const pair_t ys[] = {{1, 2}, {-2, 3}, {-10, 4}, {0, 7}};

  size_t n = sizeof(ys) / sizeof(ys[0]);
  for (size_t l = 0; l < n; ++l) {
    GV_STACK_CLEAR(&xs);

    // setup this stack and reverse it
    for (size_t i = 0; i < l; ++i) {
      GV_STACK_APPEND(&xs, ys[i]);
    }
    GV_STACK_REVERSE(&xs);

    // contents should be reversed
    assert(GV_STACK_SIZE(&xs) == l);
    for (size_t i = 0; i < l; ++i) {
      assert(GV_STACK_GET(&xs, i).x == ys[l - 1 - i].x);
      assert(GV_STACK_GET(&xs, i).y == ys[l - 1 - i].y);
    }
  }
  GV_STACK_FREE(&xs);
}

static void test_free(void) {
  GV_STACK(int) xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    GV_STACK_APPEND(&xs, (int)i);
  }

  GV_STACK_FREE(&xs);
  assert(GV_STACK_SIZE(&xs) == 0);
  assert(xs.impl.next_slot == xs.impl.base);
}

static void test_push_back(void) {
  GV_STACK(int) xs = {0};
  GV_STACK(int) ys = {0};

  for (size_t i = 0; i < 10; ++i) {
    GV_STACK_APPEND(&xs, (int)i);
    GV_STACK_PUSH_BACK(&ys, (int)i);
    assert(GV_STACK_SIZE(&xs) == GV_STACK_SIZE(&ys));
    for (size_t j = 0; j <= i; ++j) {
      assert(GV_STACK_GET(&xs, j) == GV_STACK_GET(&ys, j));
    }
  }

  GV_STACK_FREE(&ys);
  GV_STACK_FREE(&xs);
}

static void test_pop_back(void) {
  GV_STACK(int) xs = {0};

  for (size_t i = 0; i < 10; ++i) {
    GV_STACK_PUSH_BACK(&xs, (int)i);
  }
  for (size_t i = 0; i < 10; ++i) {
    assert(GV_STACK_SIZE(&xs) == 10 - i);
    int x = GV_STACK_POP_BACK(&xs);
    assert(x == 10 - (int)i - 1);
  }

  for (size_t i = 0; i < 10; ++i) {
    GV_STACK_PUSH_BACK(&xs, (int)i);
    (void)GV_STACK_POP_BACK(&xs);
    assert(GV_STACK_IS_EMPTY(&xs));
  }

  GV_STACK_FREE(&xs);
}

static void test_large(void) {
  GV_STACK(int) xs = {0};

  for (int i = 0; i < 5000; ++i) {
    GV_STACK_APPEND(&xs, i);
  }
  for (size_t i = 0; i < 5000; ++i) {
    assert(GV_STACK_GET(&xs, i) == (int)i);
  }

  GV_STACK_FREE(&xs);
}

static void test_dtor(void) {

  // setup a stack with a non-trivial destructor
  GV_STACK(char *) xs = {.dtor = GV_STACK_DTOR_FREE};

  for (size_t i = 0; i < 10; ++i) {
    char *hello = strdup("hello");
    assert(hello != NULL);
    GV_STACK_APPEND(&xs, hello);
  }

  for (size_t i = 0; i < 10; ++i) {
    assert(strcmp(GV_STACK_GET(&xs, i), "hello") == 0);
  }

  GV_STACK_FREE(&xs);
}

#ifndef BAD_TEST
#define BAD_TEST 0
#endif

#if BAD_TEST == 1
/// pushing a struct to an int stack should fail to compile
static UNUSED void test_bad_push1(void) {
  GV_STACK(int) xs = {0};

  struct foo {
    int x;
  };
  struct foo y = {0};

  GV_STACK_APPEND(&xs, y);
}
#endif

#if BAD_TEST == 2
/// pushing an int to a struct stack should fail to compile
static UNUSED void test_bad_push2(void) {
  struct foo {
    int x;
  };
  GV_STACK(struct foo) xs = {0};

  GV_STACK_APPEND(&xs, 1);
}
#endif

#if BAD_TEST == 3
/// getter of an int stack should return an int-typed value
static UNUSED void test_bad_get1(void) {
  GV_STACK(int) xs = {0};
  GV_STACK_APPEND(&xs, 1);

  struct foo {
    int x;
  };
  struct foo y UNUSED = GV_STACK_GET(&xs, 0);
}
#endif

#if BAD_TEST == 4
/// getter of an struct stack should return an struct-typed value
static UNUSED void test_bad_get2(void) {
  struct foo {
    int x;
  };
  GV_STACK(struct foo) xs = {0};

  struct foo y = {1};
  GV_STACK_APPEND(&xs, y);

  int x UNUSED = GV_STACK_GET(&xs, 0);
}
#endif

#if BAD_TEST == 5
/// `at` of an int stack should return an int-typed pointer
static UNUSED void test_bad_get1(void) {
  GV_STACK(int) xs = {0};
  GV_STACK_APPEND(&xs, 1);

  struct foo {
    int x;
  };
// upgrade incompatible pointer assignments into a compiler error
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wincompatible-pointer-types"
#endif
  struct foo *y UNUSED = GV_STACK_AT(&xs, 0);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}
#endif

#if BAD_TEST == 6
/// `at` of a struct stack should return an struct-typed pointer
static UNUSED void test_bad_get2(void) {
  struct foo {
    int x;
  };
  GV_STACK(struct foo) xs = {0};

  struct foo y = {1};
  GV_STACK_APPEND(&xs, y);

// upgrade incompatible pointer assignments into a compiler error
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wincompatible-pointer-types"
#endif
  int *x UNUSED = GV_STACK_AT(&xs, 0);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}
#endif

/// test removal does not leak memory
int main(void) {

#define RUN(t)                                                                 \
  do {                                                                         \
    printf("running test_%s... ", #t);                                         \
    fflush(stdout);                                                            \
    test_##t();                                                                \
    printf("OK\n");                                                            \
  } while (0)

  RUN(create_reset);
  RUN(init);
  RUN(init_reset);
  RUN(append);
  RUN(2734_append);
  RUN(2734_try_append);
  RUN(multiple_append);
  RUN(get);
  RUN(set);
  RUN(clear_empty);
  RUN(clear);
  RUN(push_one);
  RUN(push_then_pop_ten);
  RUN(push_then_pop_many);
  RUN(push_pop_interleaved);
  RUN(sort_empty);
  RUN(sort);
  RUN(sort_sorted);
  RUN(sort_complex);
  RUN(reserve);
  RUN(reverse_empty);
  RUN(reverse);
  RUN(reverse_complex);
  RUN(free);
  RUN(push_back);
  RUN(pop_back);
  RUN(large);
  RUN(dtor);
#undef RUN

  return EXIT_SUCCESS;
}
