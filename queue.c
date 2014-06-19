/*
  The MIT License (MIT)
  Copyright (c) 2014 Erick Elejalde & Leo Ferres
  (eelejalde|lferres)@udec.cl
  Permission is hereby granted, free of charge, to any person obtaining
  a copy of this software and associated documentation files (the
  "Software"), to deal in the Software without restriction, including
  without limitation the rights to use, copy, modify, merge, publish,
  distribute, sublicense, and/or sell copies of the Software, and to
  permit persons to whom the Software is furnished to do so, subject to
  the following conditions:
  The above copyright notice and this permission notice shall be
  included in all copies or substantial portions of the Software.
  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/			
#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>

#include "queue.h"
#include "hzrdptrs.h"

/******************************************************************************* 
* * Lock-free concurrent queue *
*******************************************************************************/

struct qnode *queue_newnode(int data) {
  struct qnode *node = malloc(sizeof(struct qnode));
  node->next = NULL;
  node->data = data;
  return node;
}

struct queue *queue_init() {
  struct queue *q = malloc(sizeof(struct queue));
  struct qnode *n = queue_newnode(-1);
  q->head = n;
  q->tail = n;
  return q;
}

void queue_enqueue(struct queue *q, int d, struct hprectype *hprect) {
  struct qnode *node = queue_newnode(d);
  struct qnode *t;
  while(true) {
    t = q->tail;
    hprect->HP[0] = t;
    if(q->tail != t) continue;
    struct qnode *next = t->next;
    if(q->tail != t) continue;
    if(next != NULL) {
      __sync_bool_compare_and_swap(&q->tail, t, next);
      continue;
    }
    if(__sync_bool_compare_and_swap(&t->next, NULL, node)) break;
  }
  __sync_bool_compare_and_swap(&q->tail, t, node);
  hprect->HP[0] = NULL;
}

int queue_dequeue(struct queue *q, struct hprectype *hprect, HP *hp) {
  struct qnode *h;
  int data;
  while(true) {
    h = q->head;
    hprect->HP[0] = h;
    if(q->head != h) continue;
    struct qnode *t = q->tail;
    struct qnode *next = h->next;
    hprect->HP[1] = next;
    if(q->head != h) continue;
    if(next == NULL) return -1;
    if(h == t) {
      __sync_bool_compare_and_swap(&q->tail, t, next);
      continue;
    }
    data = next->data;
    if(__sync_bool_compare_and_swap(&q->head, h, next)) break;
  }
  retire_node(hp, hprect, (void*)h);

  hprect->HP[0] = NULL;
  hprect->HP[1] = NULL;
  return data;
}