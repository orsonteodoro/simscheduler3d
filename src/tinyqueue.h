/*****************************************************************************
   Tiny Queue - An ultra cheap implementation of a linked list queue for 
   the C language. 
   Copyright (c) 2012 Orson Teodoro <oteodoro@ics.uci.edu>

   Dual licensed under the MIT or GPL version 2 or above  
  
 *****************************************************************************/

#ifndef TINYQUEUE_H
#define TINYQUEUE_H

typedef struct tinyqueuenode TINYQUEUENODE, *PTINYQUEUENODE;
typedef struct tinyqueue TINYQUEUE, *PTINYQUEUE;

PTINYQUEUE tq_new ();
void tq_free (PTINYQUEUE tq);
int tq_empty (PTINYQUEUE tq);
int tq_notempty (PTINYQUEUE tq);
void tq_enqueue (PTINYQUEUE tq, void *data);
void *tq_dequeue (PTINYQUEUE tq);
void *tq_peek (PTINYQUEUE tq);
void tq_clear (PTINYQUEUE tq);
typedef int (*PTINYQUEUEEVALFN) (void *data, void *evaldata);
void tq_foreach (PTINYQUEUE tq, PTINYQUEUEEVALFN evalfn, void *evaldata);
int tq_length (PTINYQUEUE tq);
#endif

