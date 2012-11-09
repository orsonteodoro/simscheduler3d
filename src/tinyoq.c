/*****************************************************************************
   Tiny Circular Queue - An ultra cheap implementation of a circular queue 
   for the C language.
   Copyright (c) 2012 Orson Teodoro <oteodoro@ics.uci.edu>

   Dual licensed under the MIT or GPL version 2 or above  
  
 *****************************************************************************/

//#define TEST

#ifdef TEST
#include <stdio.h>
#endif
#include <stdlib.h>
#include "tinyoq.h"

typedef void* PTINYOQUEUENODE;

struct tinyoqueue
{
  PTINYOQUEUENODE *queue;
  int first;
  int last;
  int count;
  int size;
};

PTINYOQUEUE
toq_new (int size)
{
  PTINYOQUEUE toq = calloc (1,sizeof (TINYOQUEUE));
  toq->queue = calloc (1,sizeof (PTINYOQUEUENODE) * size);
  toq->size = size;
  return toq;
}

void
toq_free (PTINYOQUEUE toq)
{
  free (toq->queue);
  free (toq);
}

void
toq_clear (PTINYOQUEUE toq)
{  
  int count = 0;
  int first = 0;
  int last = 0;
}

int
toq_empty (PTINYOQUEUE toq)
{
  return !toq->count;
}

int
toq_notempty (PTINYOQUEUE toq)
{
  return toq->count;
}

int
toq_full (PTINYOQUEUE toq)
{
  return toq->count >= toq->size;
}

int
toq_length (PTINYOQUEUE toq)
{
  return toq->count;
}

void
toq_enqueue (PTINYOQUEUE toq, void *data)
{
  toq->queue[toq->last] = data;
  toq->last = ++toq->last % toq->size;
  toq->count++;
}

void*
toq_dequeue (PTINYOQUEUE toq)
{
  void *d = toq->queue[toq->first];
  toq->first = ++toq->first % toq->size;
  toq->count--;
  return d;
}

void* 
toq_peek (PTINYOQUEUE toq)
{
  return toq->queue[toq->first];
}

//works only if new size is >= old size
void
toq_resize (PTINYOQUEUE toq, int size)
{
  toq->queue = realloc (toq->queue, sizeof (PTINYOQUEUENODE)*size);
  toq->size = size;
}

void
toq_foreach (PTINYOQUEUE toq, PTINYOQUEUEEVALFN evalfn, void *evaldata)
{
  int i;
  int pos = toq->first;
  for (i = 0; i < toq->count; i++, pos = pos++ % toq->size)
    {
      if (!evalfn (toq->queue[pos], evaldata))
        return;
    }  
}

#ifdef TEST

int
toq_test ()
{
  PTINYOQUEUE toq = toq_new(200);

  int *d;
  int i;
  int v[100];

  for (i=0;i<100;i++)
    v[i] = i;

  for (i=0;i<50;i++)
    toq_enqueue (toq,v+i);

  for (i=0;i<50;i++)
    {
      d = toq_dequeue (toq); 
      printf ("%d ", *d);
    }

  for (i=0;i<50;i++)
    toq_enqueue (toq,v+i);

  for (i=0;i<50;i++)
    {
      d = toq_dequeue (toq); 
      printf ("%d ", *d);
    }

  for (i=0;i<50;i++)
    toq_enqueue (toq,v+i);

  for (i=0;i<50;i++)
    {
      d = toq_dequeue (toq); 
      printf ("%d ", *d);
    }

  for (i=0;i<50;i++)
    toq_enqueue (toq,v+i);

  for (i=0;i<50;i++)
    {
      d = toq_dequeue (toq); 
      printf ("%d ", *d);
    }
  
  toq_free (toq);
}

int
main (int argc, int argv)
{
  toq_test ();
}

#endif
