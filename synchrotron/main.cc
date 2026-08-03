#include <cstdio>
#include <mpi.h>
#include <unistd.h>

#include "Synchrotron.h"

int main(int argc, char **argv)
{
  MPI_Init(&argc,&argv);
  int rank = MPI_PROC_NULL;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  Synchrotron sync;

  int usage = 0;
  if (rank == 0) {
    const char *const cmd = argv[0];
    for (char c = 0; c != -1; c = getopt(argc,argv,":hbi:")) {
      switch(c) {
        case '?':
          fprintf(stderr,"ERROR: %s unknown argument '-%c'\n",cmd,optopt);
          usage++;
          break;
        case 'h':
          usage++;
          break;
        case 'b':
          sync.barrier = true;
          break;
        case 'i':
          if (sscanf(optarg,"%d",&sync.iters) != 1) {
            fprintf(stderr,"ERROR: %s bad argument '-i %s'\n",cmd,optarg);
            usage++;
          }
          break;
      }
    }

    if (usage) {
      fprintf(stderr,"Usage: %s [-h] | [-b] [-i <number>]\n",cmd);
      fprintf(stderr,"\t-h\t print this usage message and exit\n");
      fprintf(stderr,"\t-b\t add MPI_Barrier before each MPI_Allreduce\n");
      fprintf(stderr,"\t-i\t number of iterations\n");
      fflush(stderr);
    }
  }
  MPI_Bcast(&usage,1,MPI_INT,0,MPI_COMM_WORLD);
  if (usage) {
    MPI_Finalize();
    return usage;
  }

  MPI_Bcast(&sync,sizeof(sync),MPI_BYTE,0,MPI_COMM_WORLD);
  sync.run();
  MPI_Finalize();
  return 0;
}
