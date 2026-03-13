/// @file
/// @brief basic unit tester for list.h

#ifdef NDEBUG
#error "this is not intended to be compiled with assertions off"
#endif

#define NO_CONFIG // suppress include of config.h in list.c

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <util/list.c>
#include <util/list.h>
#include <util/prisize_t.h>
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

/// append should not be affected by surprising parameter expansion order
/// https://gitlab.com/graphviz/graphviz/-/issues/2734
static void test_2734_append(void) {
  LIST(int) xs = {0};

  LIST_APPEND(&xs, 42);
  LIST_APPEND(&xs, LIST_GET(&xs, LIST_SIZE(&xs) - 1));
  assert(LIST_GET(&xs, 1) == 42);

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

/// try-append should not be affected by surprising parameter expansion order
/// https://gitlab.com/graphviz/graphviz/-/issues/2734
static void test_2734_try_append(void) {
  LIST(int) xs = {0};

  if (!LIST_TRY_APPEND(&xs, 42)) {
    goto done;
  }

  if (!LIST_TRY_APPEND(&xs, LIST_GET(&xs, LIST_SIZE(&xs) - 1))) {
    goto done;
  }

  assert(LIST_GET(&xs, 1) == 42);

done:
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

/// prepend should not be affected by surprising parameter expansion order
/// https://gitlab.com/graphviz/graphviz/-/issues/2734
static void test_2734_prepend(void) {
  LIST(int) xs = {0};

  LIST_PREPEND(&xs, 42);
  LIST_PREPEND(&xs, LIST_GET(&xs, 0));
  assert(LIST_GET(&xs, 0) == 42);

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

/// reverse on an empty list should be a no-op
static void test_reverse_empty(void) {
  LIST(int) xs = {0};
  LIST_REVERSE(&xs);
  assert(LIST_SIZE(&xs) == 0);
  LIST_FREE(&xs);
}

static void test_reverse(void) {
  LIST(int) xs = {0};

  // a list of ints in an arbitrary order
  const int ys[] = {4, 2, 10, 5, -42, 3};
  size_t n = sizeof(ys) / sizeof(ys[0]);
  for (size_t l = 0; l < n; ++l) {
    LIST_CLEAR(&xs);

    // setup this list and reverse it
    for (size_t i = 0; i < l; ++i) {
      LIST_APPEND(&xs, ys[i]);
    }
    LIST_REVERSE(&xs);

    // contents should be reversed
    assert(LIST_SIZE(&xs) == l);
    for (size_t i = 0; i < l; ++i) {
      assert(LIST_GET(&xs, i) == ys[l - 1 - i]);
    }
  }
  LIST_FREE(&xs);
}

/// reversing a complex type should move entire values together
static void test_reverse_complex(void) {
  LIST(pair_t) xs = {0};

  const pair_t ys[] = {{1, 2}, {-2, 3}, {-10, 4}, {0, 7}};

  size_t n = sizeof(ys) / sizeof(ys[0]);
  for (size_t l = 0; l < n; ++l) {
    LIST_CLEAR(&xs);

    // setup this list and reverse it
    for (size_t i = 0; i < l; ++i) {
      LIST_APPEND(&xs, ys[i]);
    }
    LIST_REVERSE(&xs);

    // contents should be reversed
    assert(LIST_SIZE(&xs) == l);
    for (size_t i = 0; i < l; ++i) {
      assert(LIST_GET(&xs, i).x == ys[l - 1 - i].x);
      assert(LIST_GET(&xs, i).y == ys[l - 1 - i].y);
    }
  }
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

// Helpers for copy tests

typedef LIST(int) int_list_t;
typedef LIST(char) char_list_t;

#define DEFINE_LIST_COMPARE(elt_type)                                          \
  static bool elt_type##_list_compare(elt_type##_list_t *l1,                   \
                                      elt_type##_list_t *l2) {                 \
    if (LIST_SIZE(l1) != LIST_SIZE(l2)) {                                      \
      fprintf(stderr, "sizes %" PRISIZE_T " and %" PRISIZE_T " don't match\n", \
              LIST_SIZE(l1), LIST_SIZE(l2));                                   \
      return false;                                                            \
    }                                                                          \
    for (size_t i = 0; i < LIST_SIZE(l1); ++i) {                               \
      if (LIST_GET(l1, i) != LIST_GET(l2, i)) {                                \
        fprintf(stderr,                                                        \
                "elements l1(%" PRISIZE_T ")=%d != l2(%" PRISIZE_T ")=%d\n",   \
                i, (int)LIST_GET(l1, i), i, (int)LIST_GET(l2, i));             \
        return false;                                                          \
      }                                                                        \
    }                                                                          \
    assert(l1->dtor == l2->dtor);                                              \
    return true;                                                               \
  }									       \
  _Static_assert(true, "")

DEFINE_LIST_COMPARE(int);
DEFINE_LIST_COMPARE(char);

static int int_dtor_1_count = 0;
static int int_dtor_1_sum = 0;

static void int_dtor_1(int x) {
  ++int_dtor_1_count;
  int_dtor_1_sum += x;
}

static int int_dtor_2_count = 0;
static int int_dtor_2_sum = 0;

static void int_dtor_2(int x) {
  ++int_dtor_2_count;
  int_dtor_2_sum += x;
}

/// Test that copy doesn't delete destination elements.
static void test_copy_dtor(void) {
  int_list_t xs = {.dtor = int_dtor_1};
  for (int i = 5; i > 0; --i) {
    LIST_APPEND(&xs, i);
  }
  int_list_t ys = {.dtor = int_dtor_2};
  for (int i = 0; i < 3; ++i) {
    LIST_APPEND(&ys, i);
  }
  LIST_COPY(&ys, &xs);
  assert(int_list_compare(&xs, &ys));
  // Copy doesn't delete items in target.
  assert(int_dtor_2_count == 0);
  assert(int_dtor_2_sum == 0);

  LIST_FREE(&xs);
  assert(int_dtor_1_count == 5);
  assert(int_dtor_1_sum == 15);
  LIST_FREE(&ys);
  assert(int_dtor_1_count == 10);
  assert(int_dtor_1_sum == 30);

  // Still not deleted original ys
  assert(int_dtor_2_count == 0);
  assert(int_dtor_2_sum == 0);
}

#define MULTI_TARGET_COPY_SCENARIOS(elt_type)                                  \
  static bool multi_target_##elt_type##_copy_scenarios(                        \
      elt_type##_list_t *input) {                                              \
    elt_type##_list_t empty_copy = {0};                                        \
    LIST_COPY(&empty_copy, input);                                             \
    if (!elt_type##_list_compare(&empty_copy, input)) {                        \
      fprintf(stderr, "scenario 1 (empty_copy) failed");                       \
      return false;                                                            \
      LIST_FREE(&empty_copy);                                                  \
    }                                                                          \
                                                                               \
    LIST_COPY(&empty_copy, input);                                             \
    if (!elt_type##_list_compare(&empty_copy, input)) {                        \
      fprintf(stderr, "scenario 2 (double copy) failed");                      \
      LIST_FREE(&empty_copy);                                                  \
      return false;                                                            \
    }                                                                          \
    LIST_FREE(&empty_copy);                                                    \
                                                                               \
    elt_type##_list_t misc_list = {0};                                         \
    LIST_APPEND(&misc_list, 39);                                               \
    LIST_APPEND(&misc_list, 43);                                               \
    LIST_PREPEND(&misc_list, 17);                                              \
    LIST_COPY(&misc_list, input);                                              \
    if (!elt_type##_list_compare(&misc_list, input)) {                         \
      fprintf(stderr, "scenario 3 (copy onto small list) failed");             \
      LIST_FREE(&misc_list);                                                   \
      return false;                                                            \
    }                                                                          \
    LIST_FREE(&misc_list);                                                     \
                                                                               \
    elt_type##_list_t big_list = {0};                                          \
    for (int i = 0; i < 2 * (int)LIST_SIZE(input); ++i) {                      \
      LIST_APPEND(&big_list, 3 + i);                                           \
    }                                                                          \
    LIST_COPY(&big_list, input);                                               \
    if (!elt_type##_list_compare(&big_list, input)) {                          \
      fprintf(stderr, "scenario 4 (copy onto big list) failed");               \
      LIST_FREE(&big_list);                                                    \
      return false;                                                            \
    }                                                                          \
    LIST_FREE(&big_list);                                                      \
    return true;                                                               \
  }									       \
  _Static_assert(true, "")

#define SIMPLE_COPY_TEST(elt_type)                                             \
  static void test_copy_simple_##elt_type(void) {                              \
    elt_type##_list_t list1 = {0};                                             \
    for (elt_type i = 0; i < 8; ++i) {                                         \
      LIST_PUSH_BACK(&list1, i);                                               \
    }                                                                          \
    assert(multi_target_##elt_type##_copy_scenarios(&list1));                  \
    LIST_FREE(&list1);                                                         \
                                                                               \
    elt_type##_list_t list2 = {0};                                             \
    for (elt_type i = 0; i < 8; ++i) {                                         \
      LIST_PUSH_BACK(&list2, i);                                               \
    }                                                                          \
    for (elt_type i = 0; i < 8; ++i) {                                         \
      LIST_PREPEND(&list2, 8 + i);                                             \
    }                                                                          \
    assert(multi_target_##elt_type##_copy_scenarios(&list2));                  \
    LIST_FREE(&list2);                                                         \
                                                                               \
    elt_type##_list_t list3 = {0};                                             \
    for (elt_type i = 0; i < 8; ++i) {                                         \
      LIST_PREPEND(&list3, 8 + i);                                             \
    }                                                                          \
    assert(multi_target_##elt_type##_copy_scenarios(&list3));                  \
    LIST_FREE(&list3);                                                         \
  }									       \
  _Static_assert(true, "")

MULTI_TARGET_COPY_SCENARIOS(int);
MULTI_TARGET_COPY_SCENARIOS(char);

SIMPLE_COPY_TEST(int);
SIMPLE_COPY_TEST(char);

/// Test copy with various gaps in the source list.
static void test_copy_reserve(void) {
  // With knowledge of the representation,
  // we are use `LIST_RESERVE` to control
  // the created representation to exercise
  // several buffer fill scenarios that
  // lead to different copy paths.

  // contents at start of buffer, gap at end
  int_list_t list1 = {0};
  LIST_RESERVE(&list1, 16);
  // list1.impl.capacity = 16
  for (int i = 0; i < 8; ++i) {
    LIST_PUSH_BACK(&list1, i);
  }
  // list1.base[] = { 0, 1, 2, .., 7, 0, 0, .. 0 }
  // list1.impl.head = 0
  // list1.impl.size = 8
  assert(multi_target_int_copy_scenarios(&list1));
  LIST_FREE(&list1);

  // contents filling buffer completely,
  // but with head in middle
  // will have 15,..,8,0..7 [filling buffer]
  int_list_t list2 = {0};
  LIST_RESERVE(&list2, 16);
  // list1.impl.capacity = 16
  for (int i = 0; i < 8; ++i) {
    LIST_PUSH_BACK(&list2, i);
  }
  for (int i = 0; i < 8; ++i) {
    LIST_PREPEND(&list2, 8 + i);
  }
  // list2.base[] = { 0, 1, .., 7, 15, 14, .., 8 }
  // list2.impl.head = 8
  assert(multi_target_int_copy_scenarios(&list2));
  LIST_FREE(&list2);

  // contents at end of buffer, with gap in front
  // will have 15,..,8 [packed at end of buffer]
  int_list_t list3 = {0};
  LIST_RESERVE(&list3, 16);
  // list1.impl.capacity = 16
  for (int i = 0; i < 8; ++i) {
    LIST_PREPEND(&list3, 8 + i);
  }
  // list3.base[] = { 0, 0, .., 0, 15, 14, .., 8 }
  // list2.impl.head = 8
  assert(multi_target_int_copy_scenarios(&list3));
  LIST_FREE(&list3);
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

/// test use of a list with zero-sized items
static void test_zero_item_size(void) {
#ifdef __GNUC__  // Clang or GCC
  struct foo {}; // zero-sized type, Clang/GCC extension
  LIST(struct foo) xs = {0};

  struct foo y;
  LIST_APPEND(&xs, y);
  LIST_APPEND(&xs, y);
  LIST_APPEND(&xs, y);

  LIST_FREE(&xs);
#endif
}

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
  RUN(prepend_0);
  RUN(2734_try_append);
  RUN(append_prepend);
  RUN(2734_prepend);
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
  RUN(reverse_empty);
  RUN(reverse);
  RUN(reverse_complex);
  RUN(shrink);
  RUN(shrink_empty);
  RUN(free);
  RUN(push_back);
  RUN(pop_back);
  RUN(large);
  RUN(detach);
  RUN(dtor);
  RUN(remove_with_dtor);
  RUN(zero_item_size);
  RUN(copy_dtor);
  RUN(copy_reserve);
  RUN(copy_simple_int);
  RUN(copy_simple_char);

#undef RUN

  return EXIT_SUCCESS;
}
