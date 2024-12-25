/// \file
/// \brief stub application for testing libpathplan’s installed CMake target
///
/// See ../test_regression.py::test_2598. Note, this code is only expected to be
/// compilable, not to run and give a reasonable result.

// confirm we can #include libpathplan
#include <pathplan.h>

int main(void) {
  // confirm we can use something from libpathplan
  Ppolyline_t p = {0};
  make_polyline(p, 0);
  return 0;
}
