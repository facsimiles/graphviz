/// @file
/// @brief Implementation of random number generation functionality

#include <assert.h>
#include <stdlib.h>
#include <util/random.h>

int gv_random(int bound) {
  assert(bound > 0);
  return rand() % bound;
}
