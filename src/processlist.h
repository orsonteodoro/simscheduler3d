
/******************************************************************************
   Scheduler Project
   Orson Teodoro
   17051645
   143A
  
   For educational purposes only.
 *****************************************************************************/

#ifndef PROCESSLIST_H
#define PROCESSLIST_H

typedef struct processlist PROCESSLIST, *PPROCESSLIST;
PPROCESSLIST processlist_new (int size);
void processlist_free (PPROCESSLIST pl);
void* processlist_get (PPROCESSLIST pl, int pid);
void processlist_put (PPROCESSLIST pl, int pid, void *process);
void processlist_del (PPROCESSLIST pl, int pid);
int processlist_length (PPROCESSLIST pl);

typedef int (*PROCESSLISTEVALFN) (int pid, void *process, void *evaldata);

void processlist_foreach (PPROCESSLIST pl, PROCESSLISTEVALFN evalfn, void *evaldata);

#endif
