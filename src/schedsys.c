/******************************************************************************
   Scheduler Project
   Orson Teodoro
   17051645
   143A
  
   For educational purposes only.
 *****************************************************************************/

#define DEBUG  //enable slow printf
#define FDEBUG //enable slow fprintf

#include <stdio.h>
#ifdef __LINUX__
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#ifdef HAVE_SDL
#include <SDL.h>
#include <SDL_opengl.h>
#include "schedsysgl.h"
#endif

#include "schedsys.h"
#include "processlist.h"
#include "tinylist.h"
#include "main.h"
#include "process.h"

int nterm;
int stop;

typedef void (*PSCHEDFN) (PSCHEDSYS ss);
typedef void (*PSCHEDFNVISSTEP) (PSCHEDSYS ss, void *data);
typedef void (*PSCHEDFNVISDONE) ();

PSCHEDFN step_schedfn;
#ifdef HAVE_SDL
PSCHEDFNVISSTEP vis_step = NULL;
PSCHEDFNVISDONE vis_done = NULL;
void *data;
#endif

#ifdef __LINUX__
inline void schedsys_dummy() __attribute__((always_inline));
#endif
void
schedsys_dummy() {}

#ifdef DEBUG
#define SCHEDSYS_DEBUG(fmt,...) \
printf(fmt,##__VA_ARGS__)
#else
#define SCHEDSYS_DEBUG(fmt,...) \
schedsys_dummy()
#endif

#ifdef FDEBUG
#define SCHEDSYS_FDEBUG(file,fmt,...) \
fprintf(file,fmt,##__VA_ARGS__)
#else
#define SCHEDSYS_FDEBUG(file,fmt,...) \
schedsys_dummy()
#endif

PSCHEDSYS
schedsys_new ()
{
  PSCHEDSYS ss = calloc (1, sizeof (SCHEDSYS));
  ss->new = toq_new (MAXPIDS);
  ss->waiting = toq_new (MAXPIDS);

  ss->ready = toq_new (MAXPIDS);
  ss->mlfqready[0] = toq_new (MAXPIDS);
  ss->mlfqready[1] = toq_new (MAXPIDS);
  ss->mlfqready[2] = toq_new (MAXPIDS);
  ss->mlfqready[3] = toq_new (MAXPIDS);

  ss->timer = -1;

  ss->processes = processlist_new (MAXPIDS);

  return ss;
}

void
schedsys_free (PSCHEDSYS ss)
{
  toq_free (ss->new);
  toq_free (ss->waiting);
 
  toq_free (ss->ready);
  toq_free (ss->mlfqready[0]);
  toq_free (ss->mlfqready[1]);
  toq_free (ss->mlfqready[2]);
  toq_free (ss->mlfqready[3]);

  processlist_free(ss->processes);

  free (ss);
}

int 
process_addtosystem (int pid, void *process, void *data)
{
  PPROCESS p = process;
  PSCHEDSYS ss = data;

  if (p->arrival_time == ss->timer)
    {
      toq_enqueue (ss->new, p);
      schedsys_update_ogl(ss);
    }

  return 1;
}

int
schedsys_pidsort (void *left, void *right)
{
  return *(int*)left - *(int*)right;
}

int
schedsys_renice_mlfq (void *key, void *value, void *evaldata)
{
  PSCHEDSYS ss = evaldata;
  PPROCESS p = value;
  printf("%s reniced\n", p->name);
  p->mlfqdclk[3] = MLFQ_RRQ3D;
  p->mlfqdclk[2] = MLFQ_RRQ2D;
  p->mlfqdclk[1] = MLFQ_RRQ1D;
  p->mlfqdclk[0] = 0;
  p->retlvl = 3;
  toq_enqueue (ss->mlfqready[3], p);
  return 1;
}

int
schedsys_renice_blocking_mlfq (void *key, void *value, void *evaldata)
{
  PSCHEDSYS ss = evaldata;
  PPROCESS p = value;
  printf("%s reniced\n", p->name);
  p->retlvl = 3;
  toq_enqueue (ss->waiting, p);
  return 1;
}

int
schedsys_reset_processes (int pid, void *process, void *data)
{
  PPROCESS p = process;
  process_reset (p);
  return 1;
}

int
schedsys_dump_process_stats (int pid, void *process, void *data)
{
  PPROCESS p = process;
  SCHEDSYS_FDEBUG (fout,"%d %s f:%d w:%d t:%d\n", p->pid, p->name, p->finish_time, p->waiting_time, p->turnaround_time);
  return 1;
}

int
schedsys_sum_process_stats (int pid, void *process, void *data)
{
  PPROCESS p = process;
  PSCHEDSYS_ALGSTAT algstat = data;
  algstat->avg_wait_time += p->waiting_time;
  algstat->avg_turnaround_time += p->turnaround_time;
  return 1;
}

void
schedsys_dump_alg_stats (PSCHEDSYS ss)
{
  int l;
  l = processlist_length (ss->processes);
  processlist_foreach (ss->processes, schedsys_sum_process_stats, &ss->algstat);
  ss->algstat.avg_wait_time /= l;
  ss->algstat.avg_turnaround_time /= l;
  ss->algstat.thoughput = (float)l / (ss->algstat.last_time - ss->algstat.first_time) * 1000.0f;
  SCHEDSYS_FDEBUG (fout, "wait:%.3f turn:%.3f thru:%.3f", ss->algstat.avg_wait_time, ss->algstat.avg_turnaround_time, ss->algstat.thoughput);
}

int
schedsys_tick_waiting (void *data, void *evaldata)
{
  PPROCESS p = data;
  SCHEDSYS_DEBUG("  ticked waiting for %s\n", p->name);
  p->waiting_time++;
  return 1;
}

int
schedsys_tick_demote (void *data, void *evaldata)
{
  PPROCESS p = data;
  p->mlfqdclk[p->retlvl]--;
  SCHEDSYS_DEBUG ("  %s demote clk to %d on mlfqready[%d]\n", p->name, p->mlfqdclk[p->retlvl], p->retlvl);
  return 1;
}

void
schedsys_update_ogl(PSCHEDSYS ss)
{
#ifdef HAVE_SDL
  if (vis)
    {
      SDL_Event event;

      while (SDL_PollEvent(&event))
	if (event.type == SDL_QUIT) stop=1;

      if (vis_step) vis_step (ss, data);
#ifdef __LINUX__
      usleep(50000);
      //usleep(5000);
#endif
    }
#endif
}

void
schedsys_step_fcfs (PSCHEDSYS ss)
{
  // fyi the master clock has ticked and starts at 0

  // add to long term queue
  processlist_foreach (ss->processes, process_addtosystem, ss);

  // new -> ready
  while (!toq_full (ss->ready) && toq_notempty (ss->new))
    {
      PPROCESS p; 
      p = toq_dequeue (ss->new);
      p->clk = p->cio[0];
      toq_enqueue (ss->ready, p), SCHEDSYS_DEBUG ("  new->ready\n");
      if (ss->algstat.first_time == -1)
        ss->algstat.first_time = ss->timer;
    }

  // waiting -> ready
  if (toq_notempty (ss->waiting))
    {
      PPROCESS p;
      int i = 0;
      int l = toq_length (ss->waiting);
      for (; i < l; i++)
        {
          p = toq_dequeue (ss->waiting);
          if (!p->clk)
            p->cioidx++, toq_enqueue (ss->ready, p);         //io done
          else
            toq_enqueue (ss->waiting, p);                    //still waiting
        }
    }

  // (ready -> cpu)
  if (!ss->cpu && toq_notempty (ss->ready))
    {
      ss->cpu = toq_dequeue (ss->ready), SCHEDSYS_DEBUG ("  FCFS ready->cpu\n");
      if (!statsonly)
          SCHEDSYS_FDEBUG (fout, "(%d,%d,%s) ",ss->timer,ss->cpu->pid,ss->cpu->name),
          SCHEDSYS_DEBUG ("  (%d,%d,%s)\n",ss->timer,ss->cpu->pid,ss->cpu->name);
      schedsys_update_ogl(ss);
    }

  // tick waiting time
  toq_foreach (ss->ready, schedsys_tick_waiting, ss);

  // idle condition
  if (!statsonly && ss->algstat.idle && !ss->algstat.wasidle && !ss->cpu)
    {
      SCHEDSYS_FDEBUG (fout, "(%d,idle) ",ss->algstat.last_time = ss->timer),
      SCHEDSYS_DEBUG ("  (%d,idle)\n",ss->algstat.last_time = ss->timer);
      schedsys_update_ogl(ss);
      if (nterm == 0) stop = 1;
    }

  // tick io
  if (toq_notempty (ss->waiting))
    {
      int i;
      int l;
      l = toq_length (ss->waiting);
      for (i=0;i<l;i++)
        {
          PPROCESS p;
          p = toq_dequeue (ss->waiting);
          if (p->clk <= 1)
            {
              SCHEDSYS_DEBUG ("  %s has %d io ticks left\n", p->name, p->clk); 
              p->clk = p->cio[++p->cioidx];
              toq_enqueue (ss->ready, p);
            }
          else
            {
              SCHEDSYS_DEBUG ("  %s has %d io ticks left\n", p->name, p->clk); 
              p->clk--;
              toq_enqueue (ss->waiting, p);
            }
        }
    }

  // tick cpu
  if (ss->cpu && ss->cpu->cioidx % 2 == 0)
    {
      if (ss->cpu->cioidx < ss->cpu->ciolen)
        if (ss->cpu->clk <= 1) //cpu done
          {
            if (++ss->cpu->cioidx < ss->cpu->ciolen)
              {
                ss->cpu->clk = ss->cpu->cio[ss->cpu->cioidx], toq_enqueue (ss->waiting, ss->cpu), SCHEDSYS_DEBUG ("  %s is done with cpu switching to io\n", ss->cpu->name), ss->cpu = NULL;
              }
            else
              {
                nterm--,
                ss->cpu->finish_time = ss->timer+1, ss->cpu->turnaround_time = ss->cpu->finish_time - ss->cpu->arrival_time, SCHEDSYS_DEBUG ("  %s has terminated\n", ss->cpu->name), ss->cpu = NULL;
              }
          }
        else                   //still cpu
          {
	    process_run(ss->cpu); //run for x µ (microseconds)
            ss->cpu->clk--;
            SCHEDSYS_DEBUG ("  %s cpu ticked #%d\n", ss->cpu->name, ss->cpu->clk);
          }
    }


  // idle condition
  if (toq_empty (ss->ready) && toq_empty (ss->waiting) && !ss->cpu)
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 1;
  else if (!ss->cpu && toq_notempty (ss->waiting))
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 1;
  else
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 0;
}

void
schedsys_step_rr1 (PSCHEDSYS ss)
{
  // fyi the master clock has ticked and starts at 0

  // add to long term queue
  processlist_foreach (ss->processes, process_addtosystem, ss);

  // new -> ready
  while (!toq_full (ss->ready) && toq_notempty (ss->new))
    {
      PPROCESS p; 
      p = toq_dequeue (ss->new);
      p->clk = p->cio[0];
      toq_enqueue (ss->ready, p), SCHEDSYS_DEBUG ("  new->ready\n");
      if (ss->algstat.first_time == -1)
        ss->algstat.first_time = ss->timer;
    }

  // waiting -> ready
  if (toq_notempty (ss->waiting))
    {
      PPROCESS p;
      int i = 0;
      int l = toq_length (ss->waiting);
      for (; i < l; i++)
        {
          p = toq_dequeue (ss->waiting);
          if (!p->clk)
            p->cioidx++, toq_enqueue (ss->ready, p);         //io done
          else
            toq_enqueue (ss->waiting, p);                    //still waiting
        }
    }

  // (cpu -> ready)
  if (ss->clk <= 0)
    {
      if (ss->cpu)
          toq_enqueue (ss->ready, ss->cpu), ss->cpu = NULL, SCHEDSYS_DEBUG ("  rr1p cpu->ready\n");
      ss->clk = RR1Q;
    }

  // (ready -> cpu)
  if (!ss->cpu && toq_notempty (ss->ready))
    {
      ss->clk = RR1Q, ss->cpu = toq_dequeue (ss->ready), SCHEDSYS_DEBUG ("  RR1 ready->cpu\n");
      if (!statsonly)
        SCHEDSYS_FDEBUG (fout, "(%d,%d,%s) ",ss->timer,ss->cpu->pid,ss->cpu->name),
        SCHEDSYS_DEBUG ("  (%d,%d,%s)\n",ss->timer,ss->cpu->pid,ss->cpu->name);
      schedsys_update_ogl(ss);
    }

  // idle condition
  if (!statsonly && ss->algstat.idle && !ss->algstat.wasidle && !ss->cpu)
    {
      SCHEDSYS_FDEBUG (fout, "(%d,idle) ",ss->algstat.last_time = ss->timer),
      SCHEDSYS_DEBUG ("  (%d,idle)\n",ss->algstat.last_time = ss->timer);
      if (nterm == 0) stop = 1;
      schedsys_update_ogl(ss);
    }

  // tick waiting time
  toq_foreach (ss->ready, schedsys_tick_waiting, ss);

  // tick io
  if (toq_notempty(ss->waiting))
    {
      int i;
      int l;
      l = toq_length (ss->waiting);
      for (i=0;i<l;i++)
        {
          PPROCESS p;
          p = toq_dequeue (ss->waiting);
          if (p->clk <= 1)
            {
              SCHEDSYS_DEBUG ("  %s has %d io ticks left\n", p->name, p->clk); 
              p->clk = p->cio[++p->cioidx];
              toq_enqueue (ss->ready, p);
            }
          else
            {
              SCHEDSYS_DEBUG ("  %s has %d io ticks left\n", p->name, p->clk); 
              p->clk--;
              toq_enqueue (ss->waiting, p);
            }
        }
    }

  // tick cpu
  if (ss->cpu && ss->cpu->cioidx % 2 == 0)
    {
      if (ss->cpu->cioidx < ss->cpu->ciolen)
        if (ss->cpu->clk <= 1) //cpu done
          {
            if (++ss->cpu->cioidx < ss->cpu->ciolen)
              ss->cpu->clk = ss->cpu->cio[ss->cpu->cioidx], toq_enqueue (ss->waiting, ss->cpu), SCHEDSYS_DEBUG ("  %s is done with cpu switching to io\n", ss->cpu->name), ss->cpu = NULL;
            else
              {
                nterm--,
                ss->cpu->finish_time = ss->timer+1, ss->cpu->turnaround_time = ss->cpu->finish_time - ss->cpu->arrival_time, SCHEDSYS_DEBUG ("  %s has terminated\n", ss->cpu->name), ss->cpu = NULL;
              }
          }
        else                   //still cpu
          {
	    process_run(ss->cpu); //run for x µ (microseconds)
            SCHEDSYS_DEBUG ("  rr tick: %d\n", ss->clk),
            ss->clk--,
            ss->cpu->clk--,
            SCHEDSYS_DEBUG ("  ticked process... %s has %d cpu ticks left\n", ss->cpu->name, ss->cpu->clk);
          }
    }

  // idle condition
  if (toq_empty (ss->ready) && toq_empty (ss->waiting) && !ss->cpu)
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 1;
  else if (!ss->cpu && toq_notempty (ss->waiting))
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 1;
  else
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 0;
}

void
schedsys_step_rr2 (PSCHEDSYS ss)
{
  // fyi the master clock has ticked and starts at 0

  // add to long term queue
  processlist_foreach (ss->processes, process_addtosystem, ss);

  // new -> ready
  while (!toq_full (ss->ready) && toq_notempty (ss->new))
    {
      PPROCESS p; 
      p = toq_dequeue (ss->new);
      p->clk = p->cio[0];
      toq_enqueue (ss->ready, p), SCHEDSYS_DEBUG ("  new->ready\n");
      if (ss->algstat.first_time == -1)
        ss->algstat.first_time = ss->timer;
    }

  // waiting -> ready
  if (toq_notempty (ss->waiting))
    {
      PPROCESS p;
      int i = 0;
      int l = toq_length (ss->waiting);
      for (; i < l; i++)
        {
          p = toq_dequeue (ss->waiting);
          if (!p->clk)
            p->cioidx++, toq_enqueue (ss->ready, p);         //io done
          else
            toq_enqueue (ss->waiting, p); //still waiting
        }
    }

  // (cpu -> ready)
  if (ss->clk <= 0) //preempt
    {
      if (ss->cpu)
        toq_enqueue (ss->ready, ss->cpu), ss->cpu = NULL, SCHEDSYS_DEBUG ("  rr2p cpu->ready\n");
      ss->clk = RR2Q;
    }

  // (ready -> cpu)
  if (!ss->cpu && toq_notempty(ss->ready)) //nothing on cpu
    {
      ss->clk = RR2Q, ss->cpu = toq_dequeue (ss->ready), SCHEDSYS_DEBUG ("  RR2 ready->cpu\n");
      if (!statsonly)
        SCHEDSYS_FDEBUG(fout, "(%d,%d,%s) ",ss->timer,ss->cpu->pid,ss->cpu->name),
        SCHEDSYS_DEBUG("  (%d,%d,%s)\n",ss->timer,ss->cpu->pid,ss->cpu->name);
      schedsys_update_ogl(ss);
    }

  // tick waiting time
  toq_foreach (ss->ready, schedsys_tick_waiting, ss);

  // idle condition
  if (!statsonly && ss->algstat.idle && !ss->algstat.wasidle && !ss->cpu)
    {
      SCHEDSYS_FDEBUG (fout, "(%d,idle) ",ss->algstat.last_time = ss->timer),
      SCHEDSYS_DEBUG ("  (%d,idle)\n",ss->algstat.last_time = ss->timer);
      if (nterm == 0) stop = 1;
      schedsys_update_ogl(ss);
    }

  // tick io
  if (toq_notempty(ss->waiting))
    {
      int i;
      int l;
      l = toq_length (ss->waiting);
      for (i=0;i<l;i++)
        {
          PPROCESS p;
          p = toq_dequeue (ss->waiting);
          if (p->clk <= 1)
            {
              SCHEDSYS_DEBUG ("  %s has %d io ticks left\n", p->name, p->clk); 
              p->clk = p->cio[++p->cioidx];
              toq_enqueue (ss->ready, p);
            }
          else
            {
              SCHEDSYS_DEBUG ("  %s has %d io ticks left\n", p->name, p->clk); 
              p->clk--;
              toq_enqueue (ss->waiting, p);
            }
        }
    }

  // tick cpu
  if (ss->cpu && ss->cpu->cioidx % 2 == 0)
    {
      if (ss->cpu->cioidx < ss->cpu->ciolen)
        if (ss->cpu->clk <= 1) //cpu done
          {
            if (++ss->cpu->cioidx < ss->cpu->ciolen)
              ss->cpu->clk = ss->cpu->cio[ss->cpu->cioidx], toq_enqueue (ss->waiting, ss->cpu), SCHEDSYS_DEBUG ("  %s is done with cpu switching to io\n", ss->cpu->name), ss->cpu = NULL;
            else
              {
                nterm--,
                ss->cpu->finish_time = ss->timer+1, ss->cpu->turnaround_time = ss->cpu->finish_time - ss->cpu->arrival_time, SCHEDSYS_DEBUG ("  %s has terminated\n", ss->cpu->name), ss->cpu = NULL;
              }
          }
        else                   //still cpu
          {
	    process_run(ss->cpu); //run for x µ (microseconds)
            SCHEDSYS_DEBUG ("  rr tick: %d\n", ss->clk),
            ss->clk--,
            ss->cpu->clk--,
            SCHEDSYS_DEBUG ("  ticked process... %s has %d cpu ticks left\n", ss->cpu->name, ss->cpu->clk);
          }
    }

  // idle condition
  if (toq_empty (ss->ready) && toq_empty (ss->waiting) && !ss->cpu)
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 1;
  else if (!ss->cpu && toq_notempty (ss->waiting))
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 1;
  else
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 0;
}

void
schedsys_step_mlfq (PSCHEDSYS ss)
{
  // fyi the master clock has ticked and starts at 0

  // add to long term queue
  processlist_foreach (ss->processes, process_addtosystem, ss);

  // new -> mlfqready[3]
  if (ss->clrclk > 0)
    while (!toq_full (ss->mlfqready[3]) && toq_notempty (ss->new))
      {
        PPROCESS p; 
        p = toq_dequeue (ss->new);
        p->clk = p->cio[0];
        p->retlvl = 3;
        toq_enqueue (ss->mlfqready[3], p), SCHEDSYS_DEBUG ("  %s new->mlfqready[3]\n", p->name);
        if (ss->algstat.first_time == -1)
          ss->algstat.first_time = ss->timer;
      }

  // auto promote
  if (ss->clrclk <= 0)
    {
      PTINYLIST t;
      PPROCESS p = NULL;
      SCHEDSYS_DEBUG ("  promoting all\n");
      t = tl_new (schedsys_pidsort);

      while (toq_notempty(ss->mlfqready[3]))
        p = toq_dequeue (ss->mlfqready[3]), tl_put (t, &p->pid, p);
      while (toq_notempty(ss->mlfqready[2]))
        p = toq_dequeue (ss->mlfqready[2]), tl_put (t, &p->pid, p);
      while (toq_notempty(ss->mlfqready[1]))
        p = toq_dequeue (ss->mlfqready[1]), tl_put (t, &p->pid, p);
      while (toq_notempty(ss->mlfqready[0]))
        p = toq_dequeue (ss->mlfqready[0]), tl_put (t, &p->pid, p);

      while (!toq_full (ss->mlfqready[3]) && toq_notempty (ss->new) && ss->clrclk > 0)
        {
          PPROCESS p; 
          p = toq_dequeue (ss->new);
          p->clk = p->cio[0];
          p->retlvl = 3;
          toq_enqueue (ss->mlfqready[3], p), SCHEDSYS_DEBUG ("  new->mlfqready[3]\n");
        }     
 
      if (ss->cpu)
        tl_put (t, &ss->cpu->pid, ss->cpu), ss->cpu = NULL;

      //renice nonblocking
      tl_foreach (t, schedsys_renice_mlfq, ss);

      //renice blocking
      tl_clear(t);
      while (toq_notempty(ss->waiting))
        p = toq_dequeue (ss->waiting), tl_put (t, &p->pid, p);      
      tl_foreach (t, schedsys_renice_blocking_mlfq, ss);

      tl_free (t);
      ss->clk = MLFQ_RRQ3;
      ss->clrclk = 1500;
    }
  else
    SCHEDSYS_DEBUG ("  clrclk #%d\n", ss->clrclk),
    ss->clrclk--;

  // insert waiting after rr
  if (toq_notempty(ss->waiting) && (ss->clk <= 0 || !ss->cpu))
    {
      int i;
      int l;
      l = toq_length (ss->waiting);
      for (i=0;i<l;i++)
        {
          PPROCESS p;
          p = toq_dequeue (ss->waiting);

          if (p->clk <= 0)
            {
              PTINYOQUEUE hash[] = { ss->mlfqready[0], ss->mlfqready[1], ss->mlfqready[2], ss->mlfqready[3] };
              SCHEDSYS_DEBUG ("  rr detected dumping %s to waiting->mlfqready[%d] clk=%d \n", p->name, p->retlvl, p->clk);
              p->clk = p->cio[++p->cioidx];
              toq_enqueue(hash[p->retlvl], p);
            }
          else
            toq_enqueue(ss->waiting, p);
        }
    }

  // cpu -> mlfqready[x] preempt
  if (ss->clk <= 0)
    {
      if (ss->cpu)
        {
          if (ss->cpu->retlvl == 3)
            {
              if (ss->cpu->mlfqdclk[3] <= 1)
                ss->cpu->retlvl=2, ss->cpu->mlfqdclk[2] = MLFQ_RRQ2D, toq_enqueue (ss->mlfqready[2], ss->cpu), SCHEDSYS_DEBUG ("  demoting %s to lvl 2\n", ss->cpu->name);   // demote
              else
                ss->cpu->retlvl=3,                                    toq_enqueue (ss->mlfqready[3], ss->cpu), SCHEDSYS_DEBUG ("  %s cpu -> mlfqready[3]\n", ss->cpu->name); // preempt
              ss->cpu = NULL;
            }
          else if (ss->cpu->retlvl == 2)
            {
              if (ss->cpu->mlfqdclk[2] <= 1)
                ss->cpu->retlvl=1, ss->cpu->mlfqdclk[1] = MLFQ_RRQ1D, toq_enqueue (ss->mlfqready[1], ss->cpu), SCHEDSYS_DEBUG ("  demoting %s to lvl 1\n", ss->cpu->name);   // demote
              else
                ss->cpu->retlvl=2,                                    toq_enqueue (ss->mlfqready[2], ss->cpu), SCHEDSYS_DEBUG ("  %s cpu -> mlfqready[2]\n", ss->cpu->name); // preempt
              ss->cpu = NULL;
            }
          else if (ss->cpu->retlvl == 1)
            {
              if (ss->cpu->mlfqdclk[1] <= 1)
                ss->cpu->retlvl=0,                                    toq_enqueue (ss->mlfqready[0], ss->cpu), SCHEDSYS_DEBUG ("  demoting %s to lvl 0\n", ss->cpu->name);   // demote
              else
                ss->cpu->retlvl=1,                                    toq_enqueue (ss->mlfqready[1], ss->cpu), SCHEDSYS_DEBUG ("  %s cpu -> mlfqready[1]\n", ss->cpu->name); // preempt
              ss->cpu = NULL;
            }
          else if (ss->cpu->retlvl == 0)
            {
              if (ss->cpu->mlfqdclk[0] <= 1)
                ss->cpu->retlvl=0,                                    toq_enqueue (ss->mlfqready[0], ss->cpu), SCHEDSYS_DEBUG ("  demoting %s to lvl 0\n", ss->cpu->name);   // demote
              else
                ss->cpu->retlvl=0,                                    toq_enqueue (ss->mlfqready[0], ss->cpu), SCHEDSYS_DEBUG ("  %s cpu -> mlfqready[0]\n", ss->cpu->name); // preempt
              ss->cpu = NULL;
            }
        }

        if (toq_notempty(ss->mlfqready[3]))
          ss->clk = MLFQ_RRQ3;
        else if (toq_notempty(ss->mlfqready[2]))
          ss->clk = MLFQ_RRQ2;
        else if (toq_notempty(ss->mlfqready[1]))
          ss->clk = MLFQ_RRQ1;
        else if (toq_notempty(ss->mlfqready[0]))
          ss->clk = MLFQ_RRQ0;
        else
          ss->clk = MLFQ_RRQ3; //default
    }

  // mlfqready[x] -> cpu nothing on cpu
  if (!ss->cpu)
    if (toq_notempty (ss->mlfqready[3]))
      {
        ss->cpu = toq_dequeue (ss->mlfqready[3]); 
        if (ss->cpu->mlfqdclk[3] <= 1)
          ss->clk = MLFQ_RRQ2, ss->cpu->retlvl=2, ss->cpu->mlfqdclk[2] = MLFQ_RRQ2D, SCHEDSYS_DEBUG ("  demoted %s to lvl 2\n", ss->cpu->name);   // demoted
        else
          ss->clk = MLFQ_RRQ3, SCHEDSYS_DEBUG ("  %s mlfqready[3] -> cpu\n", ss->cpu->name);
        if (!statsonly) 
          SCHEDSYS_FDEBUG (fout, "(%d,%d,%s) ",ss->timer,ss->cpu->pid,ss->cpu->name),
	  SCHEDSYS_DEBUG ("  (%d,%d,%s)\n",ss->timer,ss->cpu->pid,ss->cpu->name);
	  schedsys_update_ogl(ss);
      }
    else if (toq_notempty (ss->mlfqready[2]))
      {
        ss->cpu = toq_dequeue (ss->mlfqready[2]);
        if (ss->cpu->mlfqdclk[2] <= 1)
          ss->clk = MLFQ_RRQ1, ss->cpu->retlvl=1, ss->cpu->mlfqdclk[1] = MLFQ_RRQ1D, SCHEDSYS_DEBUG ("  demoted %s to lvl 1\n", ss->cpu->name);   // demoted
        else
          ss->clk = MLFQ_RRQ2, SCHEDSYS_DEBUG ("  %s mlfqready[2] -> cpu\n", ss->cpu->name);
        if (!statsonly) 
          SCHEDSYS_FDEBUG (fout, "(%d,%d,%s) ",ss->timer,ss->cpu->pid,ss->cpu->name),
          SCHEDSYS_DEBUG ("  (%d,%d,%s)\n",ss->timer,ss->cpu->pid,ss->cpu->name);
	  schedsys_update_ogl(ss);
      }
    else if (toq_notempty (ss->mlfqready[1]))
      {
        ss->cpu = toq_dequeue (ss->mlfqready[1]);
        if (ss->cpu->mlfqdclk[1] <= 1)
          ss->clk = MLFQ_RRQ0, ss->cpu->retlvl=0, ss->cpu->mlfqdclk[0] = MLFQ_RRQ2D, SCHEDSYS_DEBUG ("  demoted %s to lvl 0\n", ss->cpu->name);   // demoted
        else
          ss->clk = MLFQ_RRQ1, SCHEDSYS_DEBUG ("  %s mlfqready[1] -> cpu\n", ss->cpu->name);
        if (!statsonly) 
          SCHEDSYS_FDEBUG (fout, "(%d,%d,%s) ",ss->timer,ss->cpu->pid,ss->cpu->name),
	  SCHEDSYS_DEBUG ("  (%d,%d,%s)\n",ss->timer,ss->cpu->pid,ss->cpu->name);
	schedsys_update_ogl(ss);
      }
    else if (toq_notempty (ss->mlfqready[0]))
      {
        ss->cpu = toq_dequeue (ss->mlfqready[0]);
        ss->clk = MLFQ_RRQ0, SCHEDSYS_DEBUG ("  %s mlfqready[0] -> cpu\n", ss->cpu->name);
        if (!statsonly)
          SCHEDSYS_FDEBUG (fout, "(%d,%d,%s) ",ss->timer,ss->cpu->pid,ss->cpu->name),
	  SCHEDSYS_DEBUG ("  (%d,%d,%s)\n",ss->timer,ss->cpu->pid,ss->cpu->name);
	schedsys_update_ogl(ss);
      }

  // tick waiting time
  toq_foreach (ss->mlfqready[3], schedsys_tick_waiting, ss);
  toq_foreach (ss->mlfqready[2], schedsys_tick_waiting, ss);
  toq_foreach (ss->mlfqready[1], schedsys_tick_waiting, ss);
  toq_foreach (ss->mlfqready[0], schedsys_tick_waiting, ss);

  // idle condition
  if (!statsonly && ss->algstat.idle && !ss->algstat.wasidle && !ss->cpu)
    {
      SCHEDSYS_FDEBUG (fout, "(%d,idle) ",ss->algstat.last_time = ss->timer),
      SCHEDSYS_DEBUG ("  (%d,idle)\n",ss->algstat.last_time = ss->timer);
      if (nterm == 0) stop = 1;
      schedsys_update_ogl(ss);
    }

  // tick io
  if (toq_notempty(ss->waiting))
    {
      int i;
      int l;
      l = toq_length (ss->waiting);
      for (i=0;i<l;i++)
        {
          PPROCESS p;
          p = toq_dequeue (ss->waiting);

          if (p->clk >= 1)
            {
              SCHEDSYS_DEBUG ("  %s has %d io ticks left\n", p->name, p->clk); 
              p->clk--;
              toq_enqueue (ss->waiting, p);
            }
          else
            {
              // technically the process should be placed in the ready queue but currently
              // the process sits in the wait queue until RR completes because this scheduler
              // if it didn't delay insertion of the process into the ready queue would
              // place the waiting process before the new admitted process from the new queue
            
              // tick waiting time here also for this case
              
              // also by seperating the test we increase the O complexity by from O(n) to 2O(n)

              SCHEDSYS_DEBUG ("  io done will dump %s to ready after rr completes\n", p->name);
              p->waiting_time++;
              toq_enqueue (ss->waiting, p);
            }
        }
    }

  // tick cpu
  if (ss->cpu && ss->cpu->cioidx % 2 == 0)
    {
      if (ss->cpu->cioidx < ss->cpu->ciolen)
        if (ss->cpu->clk <= 1) //cpu done
          {
            if (++ss->cpu->cioidx < ss->cpu->ciolen)
              {
                ss->cpu->clk = ss->cpu->cio[ss->cpu->cioidx], toq_enqueue (ss->waiting, ss->cpu), SCHEDSYS_DEBUG ("  %s is done with cpu switching to io %d\n", ss->cpu->name, ss->cpu->clk);
                ss->cpu = NULL;
              }
            else
              {
                nterm--,
                ss->cpu->finish_time = ss->timer+1, ss->cpu->turnaround_time = ss->cpu->finish_time - ss->cpu->arrival_time, SCHEDSYS_DEBUG ("  %s has terminated\n", ss->cpu->name), ss->cpu = NULL;
              }
          }
        else                      //still cpu
          {
	    process_run(ss->cpu); //run for x µ (microseconds)
            SCHEDSYS_DEBUG ("  rr tick: %d\n", ss->clk),
            ss->clk--,

            ss->cpu->mlfqdclk[ss->cpu->retlvl]--,
            SCHEDSYS_DEBUG ("  %s demote clk to %d on mlfqready[%d]\n", ss->cpu->name, ss->cpu->mlfqdclk[ss->cpu->retlvl], ss->cpu->retlvl);

            ss->cpu->clk--,
            SCHEDSYS_DEBUG ("  ticked process... %s has %d cpu ticks left\n", ss->cpu->name, ss->cpu->clk);
          }
    }

  // idle condition
  if (toq_empty (ss->mlfqready[3]) && toq_empty (ss->mlfqready[2]) && toq_empty (ss->mlfqready[1]) && toq_empty (ss->mlfqready[0]) && toq_empty (ss->waiting) && !ss->cpu)
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 1;
  else if (!ss->cpu && toq_notempty (ss->waiting))
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 1;
  else
    ss->algstat.wasidle = ss->algstat.idle, ss->algstat.idle = 0;
}

void
schedsys_run (PSCHEDSYS ss)
{
  nterm = processlist_length(ss->processes);
  stop = 0;

#ifdef HAVE_SDL
  if (vis)
    {
      SDL_Init (SDL_INIT_EVERYTHING);
      SDL_GL_SetAttribute (SDL_GL_RED_SIZE, 8);
      SDL_GL_SetAttribute (SDL_GL_GREEN_SIZE, 8);
      SDL_GL_SetAttribute (SDL_GL_BLUE_SIZE, 8);
      SDL_GL_SetAttribute (SDL_GL_ALPHA_SIZE, 8);
      SDL_GL_SetAttribute (SDL_GL_BUFFER_SIZE, 32);
      SDL_GL_SetAttribute (SDL_GL_DOUBLEBUFFER, 1);
      SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, 1 );
      SDL_SetVideoMode (resw,resh,32,SDL_OPENGL | (fullscreen?SDL_FULLSCREEN:0));
    }
#endif

  switch (ss->mode)
    {
    case SCHEDSYS_MODE_FCFS:
      SCHEDSYS_FDEBUG (fout, "FCFS\n");
      step_schedfn = schedsys_step_fcfs;
#ifdef HAVE_SDL
      if (vis)
        {
          data = calloc (1, sizeof (FCFS_VIS_DATA));
          gl_fcfs_step_init (ss, data);
          vis_step = gl_fcfs_step;
          vis_done = gl_fcfs_done;
        }
#endif
      break;
    case SCHEDSYS_MODE_RR1:
      SCHEDSYS_FDEBUG (fout, "RR1\n");
      step_schedfn = schedsys_step_rr1;
#ifdef HAVE_SDL
      if (vis)
        {
          data = calloc (1, sizeof (RR_VIS_DATA));
          gl_rr_step_init (ss, data);
          vis_step = gl_rr_step;
          vis_done = gl_rr_done;
        }
#endif
      break;
    case SCHEDSYS_MODE_RR2:
      SCHEDSYS_FDEBUG (fout, "RR2\n");
      step_schedfn = schedsys_step_rr2;
#ifdef HAVE_SDL
      if (vis)
        {
          data = calloc (1, sizeof (RR_VIS_DATA));
          gl_rr_step_init (ss, data);
          vis_step = gl_rr_step;
          vis_done = gl_rr_done;
        }
#endif
      break;
    case SCHEDSYS_MODE_MLFQ:
      SCHEDSYS_FDEBUG (fout, "MLFQ\n");
      step_schedfn = schedsys_step_mlfq;
#ifdef HAVE_SDL
      if (vis)
        {
          data = calloc (1, sizeof (MLFQ_VIS_DATA));
          gl_mlfq_step_init (ss, data);
          vis_step = gl_mlfq_step;
          vis_done = gl_mlfq_done;
        }
#endif
      break;
    }

  processlist_foreach (ss->processes, schedsys_reset_processes, ss);

  ss->timer = 0;
  while (stop == 0)
    {
      SCHEDSYS_DEBUG ("tick #%d\n", ss->timer);
      step_schedfn (ss);

      ss->timer++;
    }

#ifdef HAVE_SDL
  if (vis_done) vis_done (data);
#endif

  if (!statsonly)
    SCHEDSYS_FDEBUG (fout, "\n");
  processlist_foreach (ss->processes, schedsys_dump_process_stats, ss);
  schedsys_dump_alg_stats (ss);

#ifdef HAVE_SDL
    if (vis)
      SDL_Quit ();
#endif
    SCHEDSYS_FDEBUG (fout,"\n\n");
}

void
schedsys_reset (PSCHEDSYS ss)
{
  toq_clear (ss->new);
  toq_clear (ss->waiting);
  ss->cpu = NULL;

  toq_clear (ss->ready);
  toq_clear (ss->mlfqready[0]);
  toq_clear (ss->mlfqready[1]);
  toq_clear (ss->mlfqready[2]);
  toq_clear (ss->mlfqready[3]);

  ss->clk = 0;
  ss->clrclk = 1500;

  ss->timer = 0;

  ss->algstat.idle = 1;
  ss->algstat.wasidle = 0;
  ss->algstat.first_time = -1;
  ss->algstat.last_time = -1;

  ss->algstat.thoughput = 0;
  ss->algstat.avg_wait_time = 0;
  ss->algstat.avg_turnaround_time = 0;
}

void
schedsys_setmode (PSCHEDSYS ss, int mode)
{
  switch (mode)
    {
    case SCHEDSYS_MODE_FCFS:
    case SCHEDSYS_MODE_RR1:
    case SCHEDSYS_MODE_RR2:
    case SCHEDSYS_MODE_MLFQ:
      ss->mode = mode;
      break;
    default:
      ;
    }
}
