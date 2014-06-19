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
#ifndef LIST_H_
#define LIST_H_
/*****************************************************************************/
/* List of elements */
/*****************************************************************************/
/*
  If the number of threads grows too much, the action of scanning the
  retire list of one hprec node against the HP list of every other node,
  could become inefficient. Since the HP list of all nodes combined could
  have potentially 4X the number of threads, we need and structure that
  allows fast construction and fast queries (eg. Hash Table).
  We are planning to make this improvement in future versions
*/
#define K 4 /*maximun number of hazard pointers per threads*/
#define L 256 /*initial size of the list (most be a power of 2)
                (approx. 3-4 times the expected number of threads)*/

struct listtype {
  void *list[L];
  int head;
  int tail;
};

struct listtype *list_init() ;
int list_push(struct listtype *list, void *node) ;
void *list_pop(struct listtype *list) ;
bool list_lookup(struct listtype *list, void *node) ;
int list_popall(struct listtype *list, void **output);

#endif