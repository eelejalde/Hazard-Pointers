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
*/

#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>

#include "list.h"
#include "hzrdptrs.h"

int R(int H) {
/*thershold in the number of nodes of rlist before scan*/
  return L/2;
  /*this supoused to be R(H) = H + Omega(H) if the list were to be
dynamic*/
}
void scan(HP *hp, struct hprectype *myhprec) {
  /*stage 1: scan HP lists and insert non-null values in plist*/
  struct listtype *plist = list_init();
  struct hprectype *hprec = hp->headHPRec;
  for (; hprec != NULL; hprec = hprec->next)
    for (int i = 0; i < K; i++)
      if (hprec->HP[i] != NULL)
	    list_push(plist, hprec->HP[i]);
  /*stage 2: compare plist against rlist*/
  void **tmplist = (void**)malloc(L*sizeof(void*));
  int length = list_popall(myhprec->rlist, tmplist);
  myhprec->rcount = 0;
  for (int i = 0; i < length; i++)
     if (list_lookup(plist, tmplist[i])) {
      list_push(myhprec->rlist, tmplist[i]);
      myhprec->rcount++;
    }
    else
      free(tmplist[i]);
  free(tmplist);
}

void help_scan(HP *hp, struct hprectype *myhprec) {
  struct hprectype *hprec = hp->headHPRec;
  for (; hprec; hprec = hprec->next){
    if (hprec->active ||
	!__sync_bool_compare_and_swap(&hprec->active, false, true))
      continue;
    while (hprec->rcount > 0) {
      void *node = list_pop(hprec->rlist);
      hprec->rcount--;
      list_push(myhprec->rlist, node);
      myhprec->rcount++;
      if (myhprec->rcount >= R(hp->H)) scan(hp, myhprec);
    }
    hprec->active = false;
  }
}
HP *HP_init() {
  HP *hp = malloc(sizeof(HP));
  hp->H = 0;
  hp->headHPRec = NULL;
  /*maybe we can initialize the HPRec list with a few elements */
  return hp;
}
struct hprectype *newHPRec() {
  struct hprectype *hprec = malloc(sizeof(struct hprectype));
  hprec->active = true;
  hprec->rcount = 0;
  for (int i = 0; i < K; i++)
    hprec->HP[i] = NULL;
  hprec->next = NULL;
  hprec->rlist = list_init();
  return hprec;
}
struct hprectype *allocate_HPRec(HP *hp) {
  /*first, try to get one of the inactive nodes*/
  struct hprectype *hprec = hp->headHPRec;
  for (; hprec; hprec = hprec->next)
    if (hprec->active ||
        !__sync_bool_compare_and_swap(&hprec->active, false, true))
      continue;
    else
      return hprec;

  /*increment the counter of hp*/
  __sync_fetch_and_add(&hp->H, K);

  /*create a new node and add it to the list*/
  struct hprectype *myhprec = newHPRec();
  struct hprectype *oldHead;
  do {
    oldHead = hp->headHPRec;
    myhprec->next = oldHead;
  } while (!__sync_bool_compare_and_swap(&hp->headHPRec, oldHead,
  myhprec));
  return myhprec;
}

void retire_HPRec(struct hprectype *myhprec) {
  for (int i = 0; i < K; i++)
    myhprec->HP[i] = NULL;
  myhprec->active = false;
}

void retire_node(HP *hp, struct hprectype *myhprec, void *node) {
  for (int i = 0; i < K; i++)
    if (myhprec->HP[i] == node) {
      list_push(myhprec->rlist, node);
      myhprec->rcount++;
      myhprec->HP[i] = NULL;
      if (myhprec->rcount >= R(hp->H)) {
	    scan(hp, myhprec);
	    help_scan(hp, myhprec);
      }
      break;
    }
}