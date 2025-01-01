/// @file
/// @brief Supporting file for test_c_utils.py::test_gv_find_me

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

// include the C source to get the implementation
#include <util/gv_find_me.c>

int main(void) {
  char *const me = gv_find_me();
  assert(me != NULL);
  (void)puts(me);
  free(me);
  return 0;
}
