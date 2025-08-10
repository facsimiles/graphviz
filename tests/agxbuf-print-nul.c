/// \file
/// \brief test that `agxbprint` should not account for nor append a NUL byte
///
/// This program accesses `agxbuf` internals in a way that is not expected from
/// users. This is testing the internal behavior of `agxbuf` itself.

#ifdef NDEBUG
#error "this program is not intended to be compiled with assertions disabled"
#endif

#include <assert.h>
#include <stdlib.h>
#include <util/agxbuf.h>

int main(void) {

  agxbuf xb = {0};
  assert(agxbuf_is_inline(&xb));

  // construct a string that should fit in the `agxbuf` inline storage
  char content[sizeof(xb.store) + 1] = {0};
  for (size_t i = 0; i < sizeof(content) - 1; ++i) {
    content[i] = 'A' + (char)i;
  }

  // now we should be able to print this to the buffer while remaining inline
  agxbprint(&xb, "%s", content);
  assert(agxbuf_is_inline(&xb));

  return EXIT_SUCCESS;
}
