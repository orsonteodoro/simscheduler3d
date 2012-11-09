/*****************************************************************************
   Tiny Priority Queue - An ultra cheap implementation of a priority queue 
   for the C language.
   Copyright (c) 2012 Orson Teodoro <oteodoro@ics.uci.edu>

   Dual licensed under the MIT or GPL version 2 or above  
  
 *****************************************************************************/

#ifndef TINYPRIOQ_H
#define TINYPRIOQ_H

typedef void* PTINYPRIOQNODE;

typedef int (*PTINYPRIOQCMPFN) (void *left, void *right);
typedef int (*PTINYPRIOQEVALFN) (void *data, void *evaldata);

typedef struct tinyprioq TINYPRIOQ, *PTINYPRIOQ;

typedef void (*TPQUPFN) (PTINYPRIOQ tpq, int c);
typedef void (*TPQDOWNFN) (PTINYPRIOQ tpq, int p);

typedef struct tinyprioq
{
  PTINYPRIOQNODE *heap;
  PTINYPRIOQCMPFN cmpfn;
  TPQUPFN upfn;
  TPQDOWNFN downfn;
  int count;
  int max;
  int last;
};

#define TINYPRIOQ_MAXHEAP 0
#define TINYPRIOQ_MINHEAP 1

void tpq_up_max (PTINYPRIOQ tpq, int c);
void tpq_up_min (PTINYPRIOQ tpq, int c);
void tpq_down_max (PTINYPRIOQ tpq, int p);
void tpq_down_min (PTINYPRIOQ tpq, int p);

PTINYPRIOQ tpq_new (int type, int size, PTINYPRIOQCMPFN cmpfn);
void tpq_free (PTINYPRIOQ tpq);
int tpq_full (PTINYPRIOQ tpq);
void tpq_enqueue (PTINYPRIOQ tpq, void *data);
int tpq_empty (PTINYPRIOQ tpq);
int tpq_notempty (PTINYPRIOQ tpq);
void* tpq_dequeue (PTINYPRIOQ tpq);
void* tpq_peek (PTINYPRIOQ tpq);
void tpq_resize (PTINYPRIOQ tpq, int size);
void tpq_foreach (PTINYPRIOQ tpq, PTINYPRIOQEVALFN evalfn, void *evaldata);
void tpq_clear (PTINYPRIOQ tpq);
int tpq_length (PTINYPRIOQ tpq);

#endif
