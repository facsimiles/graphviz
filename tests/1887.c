/**
 * @file
 * @brief supporting code for testing empty label behavior
 *
 * https://gitlab.com/graphviz/graphviz/-/issues/1887
 */

#include <assert.h>
#include <graphviz/cgraph.h>
#include <stddef.h>
#include <stdio.h>

int main(void) {

  Agraph_t *root = agopen("graphname", Agdirected, NULL);
  agattr_text(root, 1, "label", "");
  Agnode_t *before = agnode(root, "before", 1);
  assert(before != NULL);
  Agnode_t *after = agnode(root, "after", 1);
  assert(after != NULL);
  (void)after;
  agset(before, "label", "1");

  agwrite(root, stdout);

  agclose(root);

  return 0;
}
