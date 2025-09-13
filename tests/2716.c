/// @file
/// @brief Tester for https://gitlab.com/graphviz/graphviz/-/issues/2716

#include <graphviz/graphviz_version.h>
#include <stdio.h>

// confirm the version macros are usable at compile-time
#ifndef GRAPHVIZ_VERSION_MAJOR
#error "GRAPHVIZ_VERSION_MAJOR missing"
#endif
#ifndef GRAPHVIZ_VERSION_MINOR
#error "GRAPHVIZ_VERSION_MINOR missing"
#endif
#ifndef GRAPHVIZ_VERSION_PATCH
#error "GRAPHVIZ_VERSION_PATCH missing"
#endif

// confirm they are numeric values
static const int x = GRAPHVIZ_VERSION_MAJOR;
static const int y = GRAPHVIZ_VERSION_MINOR;
static const int z = GRAPHVIZ_VERSION_PATCH;

int main(void) {
  printf("%d.%d.%d\n", x, y, z);
  return 0;
}
