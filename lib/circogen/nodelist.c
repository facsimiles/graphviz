/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * https://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include	<circogen/nodelist.h>
#include	<circogen/circular.h>
#include	<assert.h>
#include	<limits.h>
#include	<stddef.h>
#include	<string.h>
#include	<util/list2.h>

void appendNodelist(nodelist_t *list, size_t one, Agnode_t *n) {
  assert(one <= LIST_SIZE(list));

  // expand the list by one element
  LIST_APPEND(list, NULL);

  // shuffle everything past where we will insert
  LIST_SYNC(list);
  size_t to_move = sizeof(node_t*) * (LIST_SIZE(list) - one - 1);
  if (to_move > 0) {
    memmove(LIST_AT(list, one + 1), LIST_AT(list, one), to_move);
  }

  // insert the new node
  LIST_SET(list, one, n);
}

void realignNodelist(nodelist_t *list, size_t np) {
  assert(np < LIST_SIZE(list));
  for (size_t i = np; i != 0; --i) {
    // rotate the list by 1
    node_t *const head = LIST_POP_FRONT(list);
    LIST_PUSH_BACK(list, head);
  }
}

void
insertNodelist(nodelist_t * list, Agnode_t * cn, Agnode_t * neighbor,
	       int pos)
{
  LIST_REMOVE(list, cn);

  for (size_t i = 0; i < LIST_SIZE(list); ++i) {
    Agnode_t *here = LIST_GET(list, i);
    if (here == neighbor) {
      if (pos == 0) {
        appendNodelist(list, i, cn);
      } else {
        appendNodelist(list, i + 1, cn);
      }
      break;
    }
  }
}

/// attach l2 to l1.
static void concatNodelist(nodelist_t * l1, nodelist_t * l2)
{
  for (size_t i = 0; i < LIST_SIZE(l2); ++i) {
    LIST_APPEND(l1, LIST_GET(l2, i));
  }
}

void reverseAppend(nodelist_t * l1, nodelist_t * l2)
{
    LIST_REVERSE(l2);
    concatNodelist(l1, l2);
    LIST_FREE(l2);
}

#ifdef DEBUG
void printNodelist(nodelist_t * list)
{
    for (size_t i = 0; i < LIST_SIZE(list); ++i) {
      fprintf(stderr, "%s ", agnameof(LIST_GET(list, i)));
    }
    fputs("\n", stderr);
}
#endif
