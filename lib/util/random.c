/// @file
/// @brief Implementation of random number generation functionality

#include <assert.h>
#include <stdlib.h>
#include <util/random.h>

int gv_random(int bound) {
  assert(bound > 0);

  // The interval `[0, RAND_MAX]` is not necessarily neatly divided into
  // `bound`-sized chunks. E.g. using a bound of 3 with a `RAND_MAX` of 7:
  //   ┌───┬───┬───┬───┬───┬───┬───┬───┐
  //   │ 0 │ 1 │ 2 │ 3 │ 4 │ 5 │ 6 │ 7 │
  //   └───┴───┴───┴───┴───┴───┴───┴───┘
  //    ◄─────────► ◄─────────► ◄─────►
  //     3 values    3 values   2 values
  // To guarantee a uniform distribution, derive the upper bound of the last
  // complete chunk (5 in the example above), above which we discard and
  // resample to avoid pulling from the partial trailing chunk.
  int discard_threshold = RAND_MAX;
  if (bound <= RAND_MAX) {
    discard_threshold =
        RAND_MAX - (int)(((unsigned)RAND_MAX + 1) % (unsigned)bound);
  }

  int r;
  do {
    r = rand();
  } while (r > discard_threshold);

  return r % bound;
}
