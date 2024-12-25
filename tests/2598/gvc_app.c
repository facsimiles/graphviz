/// \file
/// \brief stub application for testing libgvc’s installed CMake target
///
/// See ../test_regression.py::test_2598. Note, this code is only expected to be
/// compilable, not to run and give a reasonable result.

// confirm we can #include libgvc
#include <gvc.h>

int main(void) {
  // confirm we can use something from libgvc
  gvToggle(0);
  return 0;
}
