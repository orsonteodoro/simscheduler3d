/******************************************************************************
   Scheduler Project
   Orson Teodoro
   17051645
   143A
  
   For educational purposes only.
 *****************************************************************************/

#ifndef PROCESS_H
#define PROCESS_H

#include "tinylist.h"

typedef struct thread
{
  int threadid;
  int eax;
  int ebx;
  int ecx;
  int edx;
  int esi;
  int edi;
  int esp;
  int ebp;
  int eip;
  int eflags;

  double st0;
  double st1;
  double st2;
  double st3;
  double st4;
  double st5;
  double st6;
  double st7;

} THREAD, *PTHREAD;

typedef struct process
{
  int pid;
  char *name;
  int *cio;

  int arrival_time;
  int finish_time;
  int waiting_time;
  int turnaround_time;
  int cioidx;
  int ciolen;

  int clk;

  int mlfqdclk[4];
  int retlvl;

  PTINYLIST threads;
  int rthread;

  void *heap;
  void *stack;

} PROCESS, *PPROCESS;

PPROCESS process_new ();
void process_free (PPROCESS p);
void process_run (PPROCESS p);
void process_reset (PPROCESS p);

#endif
