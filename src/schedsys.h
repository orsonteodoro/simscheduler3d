/******************************************************************************
   Scheduler Project
   Orson Teodoro
   17051645
   143A
  
   For educational purposes only.
 *****************************************************************************/

#ifndef SCHEDSYS_H
#define SCHEDSYS_H

#include "tinyoq.h"
#include "process.h"
#include "processlist.h"

#define MAXPIDS 600

#define SCHEDSYS_MODE_FCFS    0x00010001
#define SCHEDSYS_MODE_RR1     0x00020011
#define SCHEDSYS_MODE_RR2     0x00040011
#define SCHEDSYS_MODE_MLFQ    0x00080002

#define SCHEDSYS_MASK_SINGLEQ 0x00000001
#define SCHEDSYS_MASK_MULTIQ  0x00000002
#define SCHEDSYS_MASK_RR      0x00000010

#define RR1Q 10
#define RR2Q 50

#define MLFQ_RRQ0 100
#define MLFQ_RRQ1 50
#define MLFQ_RRQ2 15
#define MLFQ_RRQ3 5

#define MLFQ_RRQ1D 100
#define MLFQ_RRQ2D 45
#define MLFQ_RRQ3D 15

typedef struct alg_stats
{
  int idle;
  int wasidle;
  int first_time;
  int last_time;

  float thoughput;
  float avg_wait_time;
  float avg_turnaround_time;
} SCHEDSYS_ALGSTAT, *PSCHEDSYS_ALGSTAT;

typedef struct schedsys
{
  PPROCESSLIST processes;
  PTINYOQUEUE new;
  PTINYOQUEUE waiting;
  PPROCESS cpu;

  //circular queues
  PTINYOQUEUE ready;
  PTINYOQUEUE mlfqready[4];

  int clk;
  int clrclk;

  int timer;
  int mode;

  SCHEDSYS_ALGSTAT algstat;
} SCHEDSYS, *PSCHEDSYS;

PSCHEDSYS schedsys_new();
void schedsys_free (PSCHEDSYS ss);
void schedsys_reset (PSCHEDSYS ss);
void schedsys_setmode (PSCHEDSYS ss, int mode);
void schedsys_run (PSCHEDSYS ss);

#endif
