/// \file
/// \brief stub application for testing libxdot’s installed CMake target
///
/// See ../test_regression.py::test_2598. Note, this code is only expected to be
/// compilable, not to run and give a reasonable result.

// confirm we can #include libxdot
#include <xdot.h>

int main(void) {
  // confirm we can use something from libxdot
  (void)parseXDot(0);
  return 0;
}
