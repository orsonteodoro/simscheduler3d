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

#include "tinyqueue.h"
#include "processlist.h"
#include "process.h"
#include "schedsys.h"

#define FIN_DELIMITER " "

int statsonly = 0;
int vis = 0;
int fcfs = 0;
int rr1 = 0;
int rr2 = 0;
int mlfq = 0;
FILE *fout;

PSCHEDSYS schedsys;

void
show_options ()
{
  printf ("Usage: SimScheduler <input_file> <output_file> [flags]\n"
          "Flags:\n"
          "  -statsonly         The output file will not contain scheduling decisions only statistics\n"
          "  -FCFS              The program will run the FCFS algorithm\n"
          "  -RR1               The program will run the RR algorithm with the first specified quanta\n"
          "  -RR2               The program will run the RR algorithm with the second specified quanta\n"
          "  -MLFQ              The program will run the MLFQ algorithm\n"
#ifdef HAVE_SDL
          "  -VIS               The program will run the simulator in multimedia mode\n"
          "  -FULLSCREEN        The program will run the visualizer in fullscreen\n"
#endif
          "\n"
          "Please report bugs to <oteodoro@uci.edu>\n");
}

void
parse_switches (int argc, char *argv[])
{
  int i = 0;
  if (argc == 1)
    {
      show_options ();
      return;
    }

  for (i=0;i<argc;i++)
    {
      if (!strcmp ("-statsonly", argv[i]))
        {
          /* The output file will not contain scheduling decisions, 
             only statistics */
          statsonly = 1;
        }
      else if (!strcmp ("-FCFS", argv[i]))      
        {
          /* i program will run the FCFS algorithm */
          fcfs = 1;
        }
      else if (!strcmp ("-RR1", argv[i]))
        {
          /* The program will run the RR algorithm with the first specified 
             quanta */
          rr1 = 1;
        }
      else if (!strcmp ("-RR2", argv[i]))
        {
          /* The program will run the RR algorithm with the second specified
             quanta */
          rr2 = 1;
        }
      else if (!strcmp ("-MLFQ", argv[i]))
        {
          /* The program will run the MLFQ algorithm. */
          mlfq = 1;
        }
#ifdef HAVE_SDL
      else if (!strcmp ("-VIS", argv[i]))
        {
          /* The program will run a multimedia simulation. */
          vis = 1;
        }
      else if (!strcmp ("-FULLSCREEN", argv[i]))
        {
          /* The program will the opengl simulation in full screen. */
		  extern int fullscreen;
          fullscreen = 1;
        }
#endif
      else if (!strcmp ("--help", argv[i]) || 
               !strcmp ("-h", argv[i]))
        {
          show_options();
          return;
        }
    }
}

void
parse_inputfile (char *filename)
{
  char buffer[80];
  char *line;
  char *ptr;
  FILE *f = fopen (filename,"r");
  
  if (f)
    {
      while ((line = fgets (buffer, 80, f)) != NULL)
        {
          PPROCESS p = process_new ();
          ptr = (char*)strtok (buffer, FIN_DELIMITER); //pid
          p->pid = (int) atoi (ptr);
          ptr = strtok (NULL, FIN_DELIMITER); //name
#ifdef _GNU_SOURCE
          asprintf(&p->name, "%s", ptr);
#else
          p->name = strdup(ptr);
#endif
          ptr = strtok (NULL, FIN_DELIMITER); //arrival_time
          p->arrival_time = (int) atoi (ptr);
          p->ciolen = 0;

          p->cio = calloc (1, sizeof(int));
          while(ptr != NULL)
            {

              ptr = strtok(NULL, FIN_DELIMITER); //cpu
              if (ptr == NULL)
                break;
              p->cio = realloc (p->cio, ++p->ciolen * sizeof (int));
              p->cio[p->ciolen-1] = (int) atoi (ptr);

              ptr = strtok (NULL, FIN_DELIMITER); //io
              if (ptr == NULL)
                break;
              p->cio = realloc (p->cio, ++p->ciolen * sizeof (int));
              p->cio[p->ciolen-1] = (int) atoi (ptr);
            }
          processlist_put (schedsys->processes, p->pid, p);
        }
      fclose (f);
    }
}

int
free_procs(int pid, void *process, void *evaldata)
{
  PPROCESS p = process;
  free (p->cio);
  free (p->name);
  process_free(p);
  return 1;
}

int
main (int argc, char **argv)
{
  schedsys = schedsys_new ();

  parse_switches (argc, argv);
  if (argc >= 2)
    parse_inputfile (argv[1]);

  if (argc >= 3)
    fout = fopen(argv[2], "w");
      
  if (fcfs)
    schedsys_reset (schedsys), schedsys_setmode (schedsys, SCHEDSYS_MODE_FCFS), schedsys_run (schedsys);
  if (rr1)
    schedsys_reset (schedsys), schedsys_setmode (schedsys, SCHEDSYS_MODE_RR1), schedsys_run (schedsys);
  if (rr2)
    schedsys_reset (schedsys), schedsys_setmode (schedsys, SCHEDSYS_MODE_RR2), schedsys_run (schedsys);
  if (mlfq)
    schedsys_reset (schedsys), schedsys_setmode (schedsys, SCHEDSYS_MODE_MLFQ), schedsys_run (schedsys);

  if (argc >= 3)
    fclose(fout);

  processlist_foreach(schedsys->processes, free_procs, NULL);
  schedsys_free(schedsys);

  return 0;
}
