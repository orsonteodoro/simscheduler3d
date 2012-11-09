/*****************************************************************************
   Tiny Priority Queue - An ultra cheap implementation of a priority queue 
   for the C language.
   Copyright (c) 2012 Orson Teodoro <oteodoro@ics.uci.edu>

   Dual licensed under the MIT or GPL version 2 or above  
  
 *****************************************************************************/

//#define TEST

#ifdef TEST
#include <stdio.h>
#endif
#include <stdlib.h>
#include "tinyprioq.h"

PTINYPRIOQ
tpq_new (int type, int size, PTINYPRIOQCMPFN cmpfn)
{
  PTINYPRIOQ tpq;
  tpq = calloc (1, sizeof (TINYPRIOQ));
  tpq->max = size;
  tpq->heap = calloc (1, sizeof (PTINYPRIOQNODE)*size);
  tpq->cmpfn = cmpfn;
       if (type == TINYPRIOQ_MAXHEAP)
    {
      tpq->upfn = tpq_up_max;
      tpq->downfn = tpq_down_max;
    }
  else if (type == TINYPRIOQ_MINHEAP)
    {
      tpq->upfn = tpq_up_min;
      tpq->downfn = tpq_down_min;
    }
  return tpq;
}

void
tpq_free (PTINYPRIOQ tpq)
{
  free(tpq->heap);
  free(tpq);
}

void
tpq_clear (PTINYPRIOQ tpq)
{
  tpq->count = 0;
  tpq->last = 0;
}

int
tpq_length (PTINYPRIOQ tpq)
{
  return tpq->count;
}

#ifdef __LINUX__
inline int tpq_parent_left (int i) __attribute__((always_inline));
inline int tpq_parent_right (int i) __attribute__((always_inline));
inline int tpq_parent (int i) __attribute__((always_inline));
inline int tpq_left_most_leaf (int n) __attribute__((always_inline));
#else
#endif
int tpq_parent_left (int i) { return i*2+1; }
int tpq_parent_right (int i) { return i*2+2; }
int tpq_parent (int i) { return (i-1)/2; }
int tpq_left_most_leaf (int n) { return n/2; }

void
tpq_xchg (PTINYPRIOQ tpq, int p, int c)
{
  PTINYPRIOQNODE t;
  t = tpq->heap[p];
  tpq->heap[p] = tpq->heap[c];
  tpq->heap[c] = t;
}

//maxheap: move large val up
void
tpq_up_max (PTINYPRIOQ tpq, int c)
{
  int p;
  if (!c)
    return;

  p = tpq_parent(c);
  if (tpq->cmpfn (tpq->heap[c], tpq->heap[p]) > 0)
    {
      tpq_xchg (tpq, p, c);
      tpq_up_max (tpq, p);
    }
}

//minheap: move small val up
void
tpq_up_min (PTINYPRIOQ tpq, int c)
{
  int p;
  if (!c)
    return;
  p = tpq_parent (c);
  if (tpq->cmpfn (tpq->heap[c], tpq->heap[p]) < 0)
    {
      tpq_xchg (tpq, p, c);
      tpq_up_min (tpq, p);
    }
}

//maxheap: move low val down
void
tpq_down_max (PTINYPRIOQ tpq, int p)
{
  int l;
  int c;

  if ((l = tpq_parent_left (p)) <= tpq->last)
    {
      if ((c = tpq_parent_right (p)) > tpq->last || tpq->cmpfn (tpq->heap[l], tpq->heap[c]) > 0)
          c = l;
      if (tpq->cmpfn (tpq->heap[p], tpq->heap[c]) < 0)
        {
          tpq_xchg (tpq, p,c);
          tpq_down_max (tpq, c);      
        }
    }
}

//minheap: move hi val down
void
tpq_down_min (PTINYPRIOQ tpq, int p)
{
  int l;
  int c;

  if ((l = tpq_parent_left (p)) <= tpq->last)
    {
      if ((c = tpq_parent_right (p)) > tpq->last || tpq->cmpfn (tpq->heap[l], tpq->heap[c]) < 0)
          c = l;
      if (tpq->cmpfn (tpq->heap[p], tpq->heap[c]) > 0)
        {
          tpq_xchg (tpq, p,c);
          tpq_down_min (tpq, c);      
        }
    } 
}

int
tpq_full (PTINYPRIOQ tpq)
{
  return tpq->count >= tpq->max;
}

void
tpq_enqueue (PTINYPRIOQ tpq, void *data)
{
  if (tpq->count == tpq->max)
    return;
  tpq->heap[tpq->count] = data;
  tpq->upfn (tpq, tpq->count);
  tpq->last = tpq->count++;
}

int
tpq_empty (PTINYPRIOQ tpq)
{
  return !tpq->count;
}

int
tpq_notempty (PTINYPRIOQ tpq)
{
  return tpq->count;
}

void*
tpq_dequeue (PTINYPRIOQ tpq)
{
  PTINYPRIOQNODE n;
  if (tpq->count == 0)
    return NULL;
  n = tpq->heap[0];
  tpq->heap[0] = tpq->heap[tpq->count-1];
  tpq->last = --tpq->count-1;
  tpq->downfn (tpq, 0);
  return n;
}

void*
tpq_peek (PTINYPRIOQ tpq)
{
  return tpq->heap[0];
}

void
tpq_resize (PTINYPRIOQ tpq, int size)
{
  tpq->max = size;
  tpq->count = (size < tpq->count) ? size : tpq->count; 
  tpq->heap = (PTINYPRIOQNODE) realloc (tpq->heap, sizeof (PTINYPRIOQNODE)*size);
}


void
tpq_infixeval (PTINYPRIOQ tpq, int p, PTINYPRIOQEVALFN evalfn, void *evaldata)
{
  int l = tpq_parent_left (p);
  int r = tpq_parent_right (p);
  if (l > tpq->last && r > tpq->last)
    return;
  tpq_infixeval (tpq, l, evalfn, evaldata);
  if (!evalfn (tpq->heap[p], evaldata))
    return;
  tpq_infixeval (tpq, r, evalfn, evaldata);
}

void
tpq_foreach (PTINYPRIOQ tpq, PTINYPRIOQEVALFN evalfn, void *evaldata)
{
  tpq_infixeval (tpq, 0, evalfn, evaldata);
}

void
tpq_foreach2 (PTINYPRIOQ tpq, PTINYPRIOQEVALFN evalfn, void *evaldata)
{
  int i;
  for(i=0;i<tpq->count;i++)
    if (!evalfn (tpq->heap[i], evaldata))
      return;
}

#ifdef TEST

int 
result (void *val, void *evaldata)
{
  int *d = val;
  printf ("%d ", *d);
  return 1;
}

int 
cmpmax (void *_left, void *_right)
{
  int *left = _left;
  int *right = _right;
  return *left-*right;
}

int
cmpmin (void *_left, void *_right)
{
  int *left = _left;
  int *right = _right;
  return *left-*right;
}

void
tpq_test_insert_max ()
{
  PTINYPRIOQ tpq = (PTINYPRIOQ)tpq_new (TINYPRIOQ_MAXHEAP, 200, cmpmax);

  int v[100];
  int i;
  for (i = 0; i < 25; i++)
    v[i] = i;

  for (i = 0; i < 25; i++)
    tpq_enqueue (tpq, v+i);

  //tpq_foreach (tpq, result, NULL);
  tpq_foreach2 (tpq, result, NULL);
  
  tpq_free (tpq);
}

void
tpq_test_insert_min ()
{
  PTINYPRIOQ tpq = (PTINYPRIOQ)tpq_new (TINYPRIOQ_MINHEAP, 200, cmpmin);

  int v[100];
  int i;

  srandom (time (NULL));

  for (i = 0; i < 25; i++)
    v[i] = random()% 100;

  for (i = 0; i < 25; i++)
    tpq_enqueue (tpq, v+i);

  //tpq_foreach (tpq, result, NULL);
  tpq_foreach2 (tpq, result, NULL);
  
  tpq_free (tpq);
}

void
tpq_test_remove_max ()
{
  PTINYPRIOQ tpq = (PTINYPRIOQ)tpq_new (TINYPRIOQ_MAXHEAP, 200, cmpmax);

  int v[100];
  int i;
  for (i = 0; i < 25; i++)
    v[i] = i;

  for (i = 0; i < 25; i++)
    tpq_enqueue (tpq, v+i);

  //tpq_foreach (tpq, result, NULL);
  //tpq_foreach2 (tpq, result, NULL);

  int *d;
  d = tpq_dequeue(tpq), printf("%d", *d);
  tpq_foreach2 (tpq, result, NULL);

  tpq_free (tpq);
}

void
tpq_test_delall_max ()
{
  PTINYPRIOQ tpq = (PTINYPRIOQ)tpq_new (TINYPRIOQ_MAXHEAP, 200, cmpmax);

  int v[100];
  int i;

  srandom (time (NULL));

  for (i = 0; i < 25; i++)
    v[i] = random()%50;

  for (i = 0; i < 25; i++)
    tpq_enqueue (tpq, v+i);

  //tpq_foreach (tpq, result, NULL);
  //tpq_foreach2 (tpq, result, NULL);

  int *d;
  for(i = 0; i < 25; i++)
    d = tpq_dequeue(tpq), printf("%d ", *d);
  tpq_foreach2 (tpq, result, NULL);

  tpq_free (tpq);
}

int
main (int argc, int argv)
{
  //tpq_test_insert_max ();
  //tpq_test_insert_min ();
  //tpq_test_remove_max ();
  tpq_test_delall_max ();
}

#endif
