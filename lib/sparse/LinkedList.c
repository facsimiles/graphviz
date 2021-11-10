/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: Details at https://graphviz.org
 *************************************************************************/

#include <sparse/LinkedList.h>
#include <common/memory.h>

SingleLinkedList SingleLinkedList_new(void *data){
  SingleLinkedList head;
  head = GNEW(struct SingleLinkedList_struct);
  head->data = data;
  head->next = NULL;
  return head;
}

SingleLinkedList SingleLinkedList_new_int(int i){
  int *data;
  data = malloc(sizeof(int));
  data[0] = i;
  return SingleLinkedList_new((void*) data);
}
  

void SingleLinkedList_delete(SingleLinkedList head,  void (*linklist_deallocator)(void*)){
  SingleLinkedList next;

  if (!head) return;
  do {
    next = head->next;
    if (head->data) linklist_deallocator(head->data);
    free(head);
    head = next;
  } while (head);

}


SingleLinkedList SingleLinkedList_prepend(SingleLinkedList l, void *data){
  SingleLinkedList head = SingleLinkedList_new(data);
  head->next = l;
  return head;
}

SingleLinkedList SingleLinkedList_prepend_int(SingleLinkedList l, int i){
  int *data;
  data = malloc(sizeof(int));
  data[0] = i;
  return SingleLinkedList_prepend(l, (void*) data);
}

void* SingleLinkedList_get_data(SingleLinkedList l){
  return l->data;
}

SingleLinkedList SingleLinkedList_get_next(SingleLinkedList l){
  return l->next;
}
void SingleLinkedList_print(SingleLinkedList head, void (*linkedlist_print)(void*)){

  if (!head) return;
  do {
    if (head->data) linkedlist_print(head->data);
    head = head->next;
  } while (head);
 
}


DoubleLinkedList DoubleLinkedList_new(void *data){
  DoubleLinkedList head;
  head = GNEW(struct DoubleLinkedList_struct);
  head->data = data;
  head->next = NULL;
  head->prev = NULL;
  return head;
}

void DoubleLinkedList_delete(DoubleLinkedList head,  void (*linklist_deallocator)(void*)){
  DoubleLinkedList next;

  if (!head) return;
  do {
    next = head->next;
    if (head->data) linklist_deallocator(head->data);
    free(head);
    head = next;
  } while (head);

}


DoubleLinkedList DoubleLinkedList_prepend(DoubleLinkedList l, void *data){
  DoubleLinkedList head = DoubleLinkedList_new(data);
  if (l){
    head->next = l;
    l->prev = head;
  }
  return head;
}

void* DoubleLinkedList_get_data(DoubleLinkedList l){
  return l->data;
}

DoubleLinkedList DoubleLinkedList_get_next(DoubleLinkedList l){
  return l->next;
}

void DoubleLinkedList_print(DoubleLinkedList head, void (*linkedlist_print)(void*)){

  if (!head) return;
  do {
    if (head->data) linkedlist_print(head->data);
    head = head->next;
  } while (head);
 
}

void DoubleLinkedList_delete_element(DoubleLinkedList l, void (*linklist_deallocator)(void*), DoubleLinkedList *head){
  /* delete an entry in the chain of linked list. If the head changes due to this (if l is the first element in the list), update */
  DoubleLinkedList next, prev;

  if (l){
    next = l->next;
    prev = l->prev;
    
    if (l->data) linklist_deallocator(l->data);
    free(l);
    l = NULL;

    if (next) next->prev = prev;
    if (prev) prev->next = next;
    if (!prev) *head = next;
  }
}
