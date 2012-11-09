/*****************************************************************************
   Tiny Queue - An ultra cheap implementation of a linked list queue for the 
   C language. 
   Copyright (c) 2012 Orson Teodoro <oteodoro@ics.uci.edu>

   Dual licensed under the MIT or GPL version 2 or above  
  
 *****************************************************************************/

//#define TEST

#ifdef TEST
#include <stdio.h>
#endif
#include <stdlib.h>
#include "tinyqueue.h"

struct tinyqueuenode
{
  void *data;
  PTINYQUEUENODE next;
};

struct tinyqueue
{
  PTINYQUEUENODE head;
  PTINYQUEUENODE tail;
};

PTINYQUEUE
tq_new ()
{
  return calloc (1,sizeof (TINYQUEUE));
}

void
tq_free (PTINYQUEUE tq)
{
  while (tq->head)
    tq_dequeue (tq);

  free (tq);
}

void
tq_clear (PTINYQUEUE tq)
{
  while (tq->head)
    tq_dequeue (tq);
}

int
tq_empty (PTINYQUEUE tq)
{
  return !tq->head;
}

int
tq_notempty (PTINYQUEUE tq)
{
  return tq->head;
}

int
tq_length (PTINYQUEUE tq)
{
  int c=0;
  PTINYQUEUENODE x=tq->head;
  while(x)
    {
      c++;
      x = x->next;
    }
  return c;
}

void
tq_enqueue (PTINYQUEUE tq, void *data)
{
  PTINYQUEUENODE n = (TINYQUEUENODE*)calloc (1, sizeof(TINYQUEUENODE));
  n->data = data;
  if (tq->tail)
    tq->tail->next = n;
  if (!tq->head)
    tq->head = n;
  tq->tail = n;
}

void*
tq_dequeue (PTINYQUEUE tq)
{
  void *d = tq->head->data;
  if (tq->tail == tq->head)
    {
      free(tq->head);
      tq->head = tq->tail = NULL;
    }
  else
    {
      PTINYQUEUENODE n = tq->head;
      tq->head = tq->head->next;
      free(n);
    }
  return d;
}

void* 
tq_peek (PTINYQUEUE tq)
{
  return tq->head->data;
}

void 
tq_foreach (PTINYQUEUE tq, PTINYQUEUEEVALFN evalfn, void *evaldata)
{
  PTINYQUEUENODE x = tq->head;
  while (x)
    {
      if (!evalfn(x->data, evaldata))
        return;
      x = x->next;
    }
}

#ifdef TEST

int
tq_test ()
{
  PTINYQUEUE tq = tq_new();
  
  int i;
  int v[100];

  for (i=0;i<100;i++)
    v[i] = i;

  for (i=0;i<100;i++)
    tq_enqueue(tq,v+i);

  int *d;
  for (i=0;i<100;i++)
    {
      d = tq_dequeue(tq); 
      printf("%d", *d);
    }

  tq_free(tq);
}

int
main (int argc, int argv)
{
  tq_test();
}

#endif
