/// @file
/// @brief supporting application for test_regression.py::test_2705

#include <assert.h>
#include <graphviz/cgraph.h>
#include <stddef.h>
#include <stdio.h>

int main(int argc, char **argv) {

  assert(argc == 3);
  (void)argc;

  // create a graph with an unlabeled node
  Agraph_t *const g1 = agopen(NULL, Agdirected, NULL);
  assert(g1 != NULL);
  Agnode_t *const n = agnode(g1, "1", 1);
  assert(n != NULL);
  (void)n;

  // write it to a file
  FILE *const out1 = fopen(argv[1], "w+");
  assert(out1 != NULL);
  {
    const int rc = agwrite(g1, out1);
    assert(rc == 0);
    (void)rc;
  }
  (void)fflush(out1);

  (void)agclose(g1);

  // parse this back in
  rewind(out1);
  Agraph_t *const g2 = agread(out1, NULL);
  assert(g2 != NULL);
  (void)fclose(out1);

  // write this one into a separate file
  FILE *const out2 = fopen(argv[2], "w");
  assert(out2 != NULL);
  {
    const int rc = agwrite(g2, out2);
    assert(rc == 0);
    (void)rc;
  }

  agclose(g2);
  (void)fclose(out2);

  return 0;
}
