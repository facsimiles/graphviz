/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <limits.h>
#include <sparse/general.h>
#include <sparse/IntStack.h>
#include <stdlib.h>

IntStack IntStack_new(void){
  IntStack s;
  size_t max_len = 1<<5;

  s = MALLOC(sizeof(struct IntStack_struct));
  s->max_len = max_len;
  s->last = SIZE_MAX;
  s->stack = MALLOC(sizeof(int)*max_len);
  return s;
}

void IntStack_delete(IntStack s){
  if (s){
    free(s->stack);
    free(s);
  }
}

static IntStack IntStack_realloc(IntStack s){
  size_t max_len = s->max_len;

  max_len += MAX(10, max_len / 5);
  s->max_len = max_len;
  s->stack = REALLOC(s->stack, sizeof(int)*max_len);
  if (!s->stack) return NULL;
  return s;
}

size_t IntStack_push(IntStack s, int i){
  // add an item and return the pos. Return SIZE_MAX if malloc failed
  if (s->last != SIZE_MAX && s->last >= s->max_len - 1) {
    if (!IntStack_realloc(s)) return SIZE_MAX;
  }
  s->stack[++s->last] = i;
  return s->last;
}
int IntStack_pop(IntStack s, int *flag){
  /* remove the last item. If none exist, return -1 */
  *flag = 0;
  if (s->last == SIZE_MAX) {
    *flag = -1;
    return -1;
  }
  return s->stack[s->last--];
}
void IntStack_print(IntStack s){
  for (size_t i = 0; i <= s->last; i++) fprintf(stderr, "%d,", s->stack[i]);
  fprintf(stderr,"\n");
}
