/******************************************************************************
   Scheduler Project
   Orson Teodoro
   17051645
   143A
  
   For educational purposes only.
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "process.h"
#include "tinylist.h"
#include "schedsys.h"
#include <signal.h>

int
PTHREAD_sort_threadid (void *left, void *right)
{
  return *(int*)right - *(int*)left;
}

PPROCESS
process_new ()
{
  PPROCESS p = calloc (1, sizeof (PROCESS));
  PTHREAD t = calloc (1, sizeof (THREAD));
  p->threads = tl_new (PTHREAD_sort_threadid);
  tl_put (p->threads, &t->threadid, t);
  return p;
}

int
PTHREAD_remove (void *key, void *value, void *evaldata)
{
  free (value);
  return 1;
}

void
process_free (PPROCESS p)
{
  tl_foreach (p->threads, PTHREAD_remove, NULL);
  tl_free (p->threads);
  free (p);
}

void
process_run (PPROCESS p)
{
  unsigned int a,b,c,d,ip;
  PTHREAD t = tl_get (p->threads, &p->rthread);
  int interrupt_signal;
  p->rthread = p->rthread++ % tl_length (p->threads);
}

void
process_reset (PPROCESS p)
{
  p->finish_time = 0;
  p->waiting_time = 0;
  p->turnaround_time = 0;
  p->cioidx = 0;
  p->clk = 0;
  p->mlfqdclk[3] = MLFQ_RRQ3D;
  p->mlfqdclk[2] = MLFQ_RRQ2D;
  p->mlfqdclk[1] = MLFQ_RRQ1D;
  p->mlfqdclk[0] = 0;
  p->retlvl = 0;
  p->rthread = 0;
}
