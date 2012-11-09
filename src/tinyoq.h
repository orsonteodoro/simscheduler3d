/*****************************************************************************
   Tiny Circular Queue - An ultra cheap implementation of a circular queue 
   for the C language.
   Copyright (c) 2012 Orson Teodoro <oteodoro@ics.uci.edu>

   Dual licensed under the MIT or GPL version 2 or above  
  
 *****************************************************************************/

#ifndef TINYOQ_H
#define TINYOQ_H

typedef struct tinyoqueue TINYOQUEUE, *PTINYOQUEUE;

PTINYOQUEUE toq_new (int size);
void toq_free (PTINYOQUEUE toq);
int toq_empty (PTINYOQUEUE toq);
int toq_notempty (PTINYOQUEUE toq);
int toq_full (PTINYOQUEUE toq);
void toq_enqueue (PTINYOQUEUE toq, void *data);
void* toq_dequeue (PTINYOQUEUE toq);
void toq_resize (PTINYOQUEUE toq, int size);
void* toq_peek (PTINYOQUEUE toq);
void toq_clear (PTINYOQUEUE toq);
typedef int (*PTINYOQUEUEEVALFN) (void *data, void *evaldata);
void toq_foreach (PTINYOQUEUE toq, PTINYOQUEUEEVALFN evalfn, void *evaldata);
int toq_length (PTINYOQUEUE toq);

#endif
