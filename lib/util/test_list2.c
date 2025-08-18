/// @file
/// @brief basic unit tester for list2.h

#ifdef NDEBUG
#error "this is not intended to be compiled with assertions off"
#endif

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/list.c>
#include <util/list2.h>
#include <util/unused.h>

// test construction and destruction, with nothing in-between
static void test_create_reset(void) {
  LIST(int) i = {0};
  LIST_FREE(&i);
}

// a list should start in a known initial state
static void test_init(void) {
  LIST(int) i = {0};
  assert(LIST_IS_EMPTY(&i));
  assert(LIST_SIZE(&i) == 0);
}

// reset of an initialized list should be OK and idempotent
static void test_init_reset(void) {
  LIST(int) i = {0};
  LIST_FREE(&i);
  LIST_FREE(&i);
  LIST_FREE(&i);
}

// repeated append
static void test_append(void) {
  LIST(int) xs = {0};
  assert(LIST_IS_EMPTY(&xs));

  for (size_t i = 0; i < 10; ++i) {
    LIST_APPEND(&xs, (int)i);
    assert(LIST_SIZE(&xs) == i + 1);
  }

  LIST_FREE(&xs);
}

/// prepend to an empty list
static void test_prepend_0(void) {
  LIST(int) xs = {0};

  LIST_PREPEND(&xs, 42);
  assert(LIST_SIZE(&xs) == 1);
  assert(LIST_GET(&xs, 0) == 42);

  LIST_FREE(&xs);
}

/// interleaved append and prepend
static void test_append_prepend(void) {
  LIST(int) xs = {0};

  for (size_t i = 0; i < 10; ++i) {
    if (i % 2 == 0) {
      LIST_APPEND(&xs, (int)i);
    } else {
      LIST_PREPEND(&xs, (int)i);
    }
  }

  assert(LIST_SIZE(&xs) == 10);
  assert(LIST_GET(&xs, 0) == 9);
  assert(LIST_GET(&xs, 1) == 7);
  assert(LIST_GET(&xs, 2) == 5);
  assert(LIST_GET(&xs, 3) == 3);
  assert(LIST_GET(&xs, 4) == 1);
  assert(LIST_GET(&xs, 5) == 0);
  assert(LIST_GET(&xs, 6) == 2);
  assert(LIST_GET(&xs, 7) == 4);
  assert(LIST_GET(&xs, 8) == 6);
  assert(LIST_GET(&xs, 9) == 8);

  LIST_FREE(&xs);
}

static void test_get(void) {
  LIST(int) xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    LIST_APPEND(&xs, (int)i);
  }

  for (size_t i = 0; i < 10; ++i) {
    assert(LIST_GET(&xs, i) == (int)i);
  }
  for (size_t i = 9;; --i) {
    assert(LIST_GET(&xs, i) == (int)i);
    if (i == 0) {
      break;
    }
  }

  LIST_FREE(&xs);
}

static void test_set(void) {
  LIST(int) xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    LIST_APPEND(&xs, (int)i);
  }

  for (size_t i = 0; i < 10; ++i) {
    LIST_SET(&xs, i, (int)(i + 1));
    assert(LIST_GET(&xs, i) == (int)i + 1);
  }
  for (size_t i = 9;; --i) {
    LIST_SET(&xs, i, (int)i - 1);
    assert(LIST_GET(&xs, i) == (int)i - 1);
    if (i == 0) {
      break;
    }
  }

  LIST_FREE(&xs);
}

/// removing from an empty list should be a no-op
static void test_remove_empty(void) {
  LIST(int) xs = {0};
  LIST_REMOVE(&xs, 10);
  assert(LIST_SIZE(&xs) == 0);
  LIST_FREE(&xs);
}

/// some basic removal tests
static void test_remove(void) {
  LIST(int) xs = {0};

  for (size_t i = 0; i < 10; ++i) {
    LIST_APPEND(&xs, (int)i);
  }

  // remove something that does not exist
  LIST_REMOVE(&xs, 42);
  for (size_t i = 0; i < 10; ++i) {
    assert(LIST_GET(&xs, i) == (int)i);
  }

  // remove in the middle
  LIST_REMOVE(&xs, 4);
  assert(LIST_SIZE(&xs) == 9);
  for (size_t i = 0; i < 9; ++i) {
    if (i < 4) {
      assert(LIST_GET(&xs, i) == (int)i);
    } else {
      assert(LIST_GET(&xs, i) == (int)i + 1);
    }
  }

  // remove the first
  LIST_REMOVE(&xs, 0);
  assert(LIST_SIZE(&xs) == 8);
  for (size_t i = 0; i < 8; ++i) {
    if (i < 3) {
      assert(LIST_GET(&xs, i) == (int)i + 1);
    } else {
      assert(LIST_GET(&xs, i) == (int)i + 2);
    }
  }

  // remove the last
  LIST_REMOVE(&xs, 9);
  assert(LIST_SIZE(&xs) == 7);
  for (size_t i = 0; i < 7; ++i) {
    if (i < 3) {
      assert(LIST_GET(&xs, i) == (int)i + 1);
    } else {
      assert(LIST_GET(&xs, i) == (int)i + 2);
    }
  }

  // remove all the rest
  for (size_t i = 0; i < 7; ++i) {
    LIST_REMOVE(&xs, LIST_GET(&xs, 0));
  }
  assert(LIST_SIZE(&xs) == 0);

  LIST_FREE(&xs);
}

static void test_at(void) {
  LIST(int) xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    LIST_APPEND(&xs, (int)i);
  }

  for (size_t i = 0; i < 10; ++i) {
    assert(LIST_GET(&xs, i) == *LIST_AT(&xs, i));
  }

  for (size_t i = 0; i < 10; ++i) {
    int *j = LIST_AT(&xs, i);
    *j = (int)i + 1;
    assert(LIST_GET(&xs, i) == (int)i + 1);
  }

  LIST_FREE(&xs);
}

static void test_clear_empty(void) {
  LIST(int) xs = {0};
  LIST_CLEAR(&xs);
  assert(LIST_IS_EMPTY(&xs));

  LIST_FREE(&xs);
}

static void test_clear(void) {
  LIST(int) xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    LIST_APPEND(&xs, (int)i);
  }

  assert(!LIST_IS_EMPTY(&xs));
  LIST_CLEAR(&xs);
  assert(LIST_IS_EMPTY(&xs));

  LIST_FREE(&xs);
}

// basic push then pop
static void test_push_one(void) {
  LIST(int) s = {0};
  int arbitrary = 42;
  LIST_PUSH_BACK(&s, arbitrary);
  assert(LIST_SIZE(&s) == 1);
  int top = LIST_POP_BACK(&s);
  assert(top == arbitrary);
  assert(LIST_IS_EMPTY(&s));
  LIST_FREE(&s);
}

static void push_then_pop(int count) {
  LIST(int) s = {0};
  for (int i = 0; i < count; ++i) {
    LIST_PUSH_BACK(&s, i);
    assert(LIST_SIZE(&s) == (size_t)i + 1);
  }
  for (int i = count - 1;; --i) {
    assert(LIST_SIZE(&s) == (size_t)i + 1);
    int p = LIST_POP_BACK(&s);
    assert(p == i);
    if (i == 0) {
      break;
    }
  }
  LIST_FREE(&s);
}

// push a series of items
static void test_push_then_pop_ten(void) { push_then_pop(10); }

// push enough to cause an expansion
static void test_push_then_pop_many(void) { push_then_pop(4096); }

// interleave some push and pop operations
static void test_push_pop_interleaved(void) {
  LIST(int) s = {0};
  size_t size = 0;
  for (int i = 0; i < 4096; ++i) {
    if (i % 3 == 1) {
      int p = LIST_POP_BACK(&s);
      assert(p == i - 1);
      --size;
    } else {
      LIST_PUSH_BACK(&s, i);
      ++size;
    }
    assert(LIST_SIZE(&s) == size);
  }
  LIST_FREE(&s);
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

/// sort on an empty list should be a no-op
static void test_sort_empty(void) {
  LIST(int) xs = {0};
  LIST_SORT(&xs, cmp_int);
  assert(LIST_SIZE(&xs) == 0);
  LIST_FREE(&xs);
}

static void test_sort(void) {
  LIST(int) xs = {0};

  // a list of ints in an arbitrary order
  const int ys[] = {4, 2, 10, 5, -42, 3};

  // setup this list and sort it
  for (size_t i = 0; i < sizeof(ys) / sizeof(ys[0]); ++i) {
    LIST_APPEND(&xs, ys[i]);
  }
  LIST_SORT(&xs, cmp_int);

  // we should now have a sorted version of `ys`
  assert(LIST_SIZE(&xs) == sizeof(ys) / sizeof(ys[0]));
  assert(LIST_GET(&xs, 0) == -42);
  assert(LIST_GET(&xs, 1) == 2);
  assert(LIST_GET(&xs, 2) == 3);
  assert(LIST_GET(&xs, 3) == 4);
  assert(LIST_GET(&xs, 4) == 5);
  assert(LIST_GET(&xs, 5) == 10);

  LIST_FREE(&xs);
}

/// sorting an already sorted list should be a no-op
static void test_sort_sorted(void) {
  LIST(int) xs = {0};
  const int ys[] = {-42, 2, 3, 4, 5, 10};

  for (size_t i = 0; i < sizeof(ys) / sizeof(ys[0]); ++i) {
    LIST_APPEND(&xs, ys[i]);
  }
  LIST_SORT(&xs, cmp_int);

  for (size_t i = 0; i < sizeof(ys) / sizeof(ys[0]); ++i) {
    assert(LIST_GET(&xs, i) == ys[i]);
  }

  LIST_FREE(&xs);
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
  LIST(pair_t) xs = {0};

  const pair_t ys[] = {{1, 2}, {-2, 3}, {-10, 4}, {0, 7}};

  for (size_t i = 0; i < sizeof(ys) / sizeof(ys[0]); ++i) {
    LIST_APPEND(&xs, ys[i]);
  }
  LIST_SORT(&xs, cmp_pair);

  assert(LIST_SIZE(&xs) == sizeof(ys) / sizeof(ys[0]));
  assert(LIST_GET(&xs, 0).x == -10);
  assert(LIST_GET(&xs, 0).y == 4);
  assert(LIST_GET(&xs, 1).x == -2);
  assert(LIST_GET(&xs, 1).y == 3);
  assert(LIST_GET(&xs, 2).x == 0);
  assert(LIST_GET(&xs, 2).y == 7);
  assert(LIST_GET(&xs, 3).x == 1);
  assert(LIST_GET(&xs, 3).y == 2);

  LIST_FREE(&xs);
}

static void test_shrink(void) {
  LIST(int) xs = {0};

  // to test this one we need to access the list internals
  while (LIST_SIZE(&xs) == xs.impl.capacity) {
    LIST_APPEND(&xs, 42);
  }

  assert(xs.impl.capacity > LIST_SIZE(&xs));
  LIST_SHRINK_TO_FIT(&xs);
  assert(xs.impl.capacity == LIST_SIZE(&xs));

  LIST_FREE(&xs);
}

static void test_shrink_empty(void) {
  LIST(int) xs = {0};
  LIST_SHRINK_TO_FIT(&xs);
  assert(xs.impl.capacity == 0);
  LIST_FREE(&xs);
}

static void test_free(void) {
  LIST(int) xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    LIST_APPEND(&xs, (int)i);
  }

  LIST_FREE(&xs);
  assert(LIST_SIZE(&xs) == 0);
  assert(xs.impl.capacity == 0);
}

static void test_push_back(void) {
  LIST(int) xs = {0};
  LIST(int) ys = {0};

  for (size_t i = 0; i < 10; ++i) {
    LIST_APPEND(&xs, (int)i);
    LIST_PUSH_BACK(&ys, (int)i);
    assert(LIST_SIZE(&xs) == LIST_SIZE(&ys));
    for (size_t j = 0; j <= i; ++j) {
      assert(LIST_GET(&xs, j) == LIST_GET(&ys, j));
    }
  }

  LIST_FREE(&ys);
  LIST_FREE(&xs);
}

static void test_pop_back(void) {
  LIST(int) xs = {0};

  for (size_t i = 0; i < 10; ++i) {
    LIST_PUSH_BACK(&xs, (int)i);
  }
  for (size_t i = 0; i < 10; ++i) {
    assert(LIST_SIZE(&xs) == 10 - i);
    int x = LIST_POP_BACK(&xs);
    assert(x == 10 - (int)i - 1);
  }

  for (size_t i = 0; i < 10; ++i) {
    LIST_PUSH_BACK(&xs, (int)i);
    (void)LIST_POP_BACK(&xs);
    assert(LIST_IS_EMPTY(&xs));
  }

  LIST_FREE(&xs);
}

static void test_large(void) {
  LIST(int) xs = {0};

  for (int i = 0; i < 5000; ++i) {
    LIST_APPEND(&xs, i);
  }
  for (size_t i = 0; i < 5000; ++i) {
    assert(LIST_GET(&xs, i) == (int)i);
  }

  LIST_FREE(&xs);
}

static void test_detach(void) {
  LIST(int) xs = {0};
  for (size_t i = 0; i < 10; ++i) {
    LIST_APPEND(&xs, (int)i);
  }

  int *ys;
  size_t ys_size;
  LIST_DETACH(&xs, &ys, &ys_size);
  assert(ys != NULL);
  assert(ys_size == 10);
  assert(LIST_IS_EMPTY(&xs));

  for (size_t i = 0; i < 10; ++i) {
    assert(ys[i] == (int)i);
  }

  free(ys);
}

static void test_dtor(void) {

  // setup a list with a non-trivial destructor
  LIST(char *) xs = {.dtor = LIST_DTOR_FREE};

  for (size_t i = 0; i < 10; ++i) {
    char *hello = strdup("hello");
    assert(hello != NULL);
    LIST_APPEND(&xs, hello);
  }

  for (size_t i = 0; i < 10; ++i) {
    assert(strcmp(LIST_GET(&xs, i), "hello") == 0);
  }

  LIST_FREE(&xs);
}

/// test removal does not leak memory
static void test_remove_with_dtor(void) {
  LIST(char *) xs = {.dtor = LIST_DTOR_FREE};

  char *hello = strdup("hello");
  assert(hello != NULL);

  LIST_APPEND(&xs, hello);
  LIST_REMOVE(&xs, hello);
  assert(LIST_SIZE(&xs) == 0);

  LIST_FREE(&xs);
}

#ifndef BAD_TEST
#define BAD_TEST 0
#endif

#if BAD_TEST == 1
/// appending a struct to an int list should fail to compile
static UNUSED void test_bad_append1(void) {
  LIST(int) xs = {0};

  struct foo {
    int x;
  };
  struct foo y = {0};

  LIST_APPEND(&xs, y);
}
#endif

#if BAD_TEST == 2
/// appending an int to a struct list should fail to compile
static UNUSED void test_bad_append2(void) {
  struct foo {
    int x;
  };
  LIST(struct foo) xs = {0};

  LIST_APPEND(&xs, 1);
}
#endif

#if BAD_TEST == 3
/// getter of an int list should return an int-typed value
static UNUSED void test_bad_get1(void) {
  LIST(int) xs = {0};
  LIST_APPEND(&xs, 1);

  struct foo {
    int x;
  };
  struct foo y UNUSED = LIST_GET(&xs, 0);
}
#endif

#if BAD_TEST == 4
/// getter of an struct list should return an struct-typed value
static UNUSED void test_bad_get2(void) {
  struct foo {
    int x;
  };
  LIST(struct foo) xs = {0};

  struct foo y = {1};
  LIST_APPEND(&xs, y);

  int x UNUSED = LIST_GET(&xs, 0);
}
#endif

#if BAD_TEST == 5
/// `at` of an int list should return an int-typed pointer
static UNUSED void test_bad_get1(void) {
  LIST(int) xs = {0};
  LIST_APPEND(&xs, 1);

  struct foo {
    int x;
  };
// upgrade incompatible pointer assignments into a compiler error
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wincompatible-pointer-types"
#endif
  struct foo *y UNUSED = LIST_AT(&xs, 0);
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
}
#endif

#if BAD_TEST == 6
/// `at` of a struct list should return an struct-typed pointer
static UNUSED void test_bad_get2(void) {
  struct foo {
    int x;
  };
  LIST(struct foo) xs = {0};

  struct foo y = {1};
  LIST_APPEND(&xs, y);

// upgrade incompatible pointer assignments into a compiler error
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic error "-Wincompatible-pointer-types"
#endif
  int *x UNUSED = LIST_AT(&xs, 0);
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
  RUN(prepend_0);
  RUN(append_prepend);
  RUN(get);
  RUN(set);
  RUN(remove_empty);
  RUN(remove);
  RUN(at);
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
  RUN(shrink);
  RUN(shrink_empty);
  RUN(free);
  RUN(push_back);
  RUN(pop_back);
  RUN(large);
  RUN(detach);
  RUN(dtor);
  RUN(remove_with_dtor);

#undef RUN

  return EXIT_SUCCESS;
}
