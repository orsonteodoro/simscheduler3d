/******************************************************************************
   Scheduler Project
   Orson Teodoro
   17051645
   143A
  
   For educational purposes only.
 *****************************************************************************/

#include <stdlib.h>
#include "processlist.h"
#include "process.h"

typedef void* PROCESSLISTNODE;

struct processlist
{
  PROCESSLISTNODE *processes;
  int size;
  int count;
};

//we use direct hashing for O(1) access who cares about collisons

PPROCESSLIST
processlist_new (int size)
{
  PPROCESSLIST pl = calloc (1, sizeof (PROCESSLIST));
  pl->processes = calloc (1, sizeof (PROCESSLISTNODE)*size);
  pl->size = size;
  return pl;
}

void
processlist_free (PPROCESSLIST pl)
{
  free (pl->processes);
  free (pl);
}

void*
processlist_get (PPROCESSLIST pl, int pid)
{
  return pl->processes[pid];
}

void
processlist_put (PPROCESSLIST pl, int pid, void *process)
{
  pl->processes[pid] = process;
}

void
processlist_del (PPROCESSLIST pl, int pid)
{
  pl->processes[pid] = NULL;
}

void
processlist_foreach (PPROCESSLIST pl, PROCESSLISTEVALFN evalfn, void *evaldata)
{
  int i;
  for (i=0;i<pl->size;i++)
      if (pl->processes[i] != NULL)
        if (!evalfn (i, pl->processes[i], evaldata))
          return;
}

int
processlist_length (PPROCESSLIST pl)
{
  int c = 0;
  int i = 0;
  for (;i<pl->size;i++)
    if (pl->processes[i] != NULL)
      c++;
  return c;
}
