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
  Based on:
  http://www.research.ibm.com/people/m/michael/ieeetpds-2004.pdf
  gcc -g -std=c99 -o hp list.c hzrdptrs.c queue.c main.c -lpthread */

#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <time.h>
#include <time.h>
#include <malloc.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include "list.h"
#include "hzrdptrs.h"
#include "queue.h"

/******************************************************************************* 
* * Test the hazard pointer with a concurrent queue *
*******************************************************************************/ 

void test1(struct queue *q, struct hprectype *hprec, HP *hp) {
  srand(time(NULL));
  for(int i = 0; i < 100000; i++) {
    //printf("Iteration %d\n", i);
    queue_enqueue(q, (int)(rand()%100), hprec);
    for(int j = 1000; j; j--);
    queue_dequeue(q, hprec, hp);
  }
}

/*par is the chance of getting an enqueuer*/
void test2(struct queue *q, struct hprectype *hprec, HP *hp, int par) {
  srand(time(NULL));
  for(int i = 0; i < 1000; i++) {
    if(rand()%100 < par)
      queue_enqueue(q, (int)(rand()%100), hprec);
    else
      queue_dequeue(q, hprec, hp);
    for(int j = 1000; j; j--);
  }
}

struct opts {
  struct queue *queue;
  HP *hp;
  int par; /* par is chance of getting an enqueuer*/
  int test; /*1 for test1, 2 for test2*/
};

void *benchmrk(void *args) {
  struct opts o = *(struct opts*)args;
  HP *hp = o.hp;
  struct hprectype *hprec = allocate_HPRec(hp);
  if(o.test == 1)
    test1(o.queue, hprec, hp);
  else
    test2(o.queue, hprec, hp, o.par);
}
int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: hp <enqueuer probability> <numthreads> <type of test>\n");
    printf("type of test: (1) 1 enqueue + 1 dequeue (2) random enqueue/dequeue\n");
    return -1;
  }
  struct opts *o = malloc(sizeof(struct opts));
  o->queue = queue_init();
  o->hp = HP_init();
  o->par = atoi(argv[1]);
  o->test = atoi(argv[3]);
  struct timeval t0, t1;
  pthread_t *t = malloc(sizeof(pthread_t) * atoi(argv[2]));
  gettimeofday(&t0, 0);
  for (int i = 0; i < atoi(argv[2]); i++)
    pthread_create(&t[i], NULL, benchmrk, (void *)o);
  for (int i = 0; i < atoi(argv[2]); i++)
    pthread_join(t[i], NULL);

  gettimeofday(&t1, 0);
  printf("time: %ld\n", (t1.tv_sec-t0.tv_sec)*1000000 + t1.tv_usec-t0.tv_usec);

  return 0;
}