/// @file
/// @brief basic unit tester for itos.h

#ifdef NDEBUG
#error this is not intended to be compiled with assertions off
#endif

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/itos.h>

static void test_0(void) { assert(strcmp(ITOS(0), "0") == 0); }

static void test_1(void) { assert(strcmp(ITOS(1), "1") == 0); }

static void test_neg_1(void) { assert(strcmp(ITOS(-1), "-1") == 0); }

static void test_min(void) {

  int r = snprintf(NULL, 0, "%lld", LLONG_MIN);
  assert(r > 0);

  char *buffer = malloc(sizeof(char) * ((size_t)r + 1));
  assert(buffer != NULL);

  sprintf(buffer, "%lld", LLONG_MIN);

  assert(strcmp(ITOS(LLONG_MIN), buffer) == 0);

  free(buffer);
}

static void test_max(void) {

  int r = snprintf(NULL, 0, "%lld", LLONG_MAX);
  assert(r > 0);

  char *buffer = malloc(sizeof(char) * ((size_t)r + 1));
  assert(buffer != NULL);

  sprintf(buffer, "%lld", LLONG_MAX);

  assert(strcmp(ITOS(LLONG_MAX), buffer) == 0);

  free(buffer);
}

int main(void) {

#define RUN(t)                                                                 \
  do {                                                                         \
    printf("running test_%s... ", #t);                                         \
    fflush(stdout);                                                            \
    test_##t();                                                                \
    printf("OK\n");                                                            \
  } while (0)

  RUN(0);
  RUN(1);
  RUN(neg_1);
  RUN(min);
  RUN(max);

#undef RUN

  return EXIT_SUCCESS;
}
