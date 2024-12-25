/// \file
/// \brief stub application for testing libcgraph’s installed CMake target
///
/// See ../test_regression.py::test_2598_1. Note, this code is only expected to
/// be compilable, not to run and give a reasonable result.

// confirm we can #include libcgraph
#include <cgraph.h>

int main(void) {
  // confirm we can use something from libcgraph
  Agdesc_t desc = {0};
  Agdisc_t disc = {0};
  (void)agopen(0, desc, &disc);
  return 0;
}
