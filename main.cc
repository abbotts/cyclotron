#include <cstdio>
#include <mpi.h>
#include <unistd.h>

#include "Cyclotron.h"
#include "gpu.h"

int main(int argc, char **argv)
{
  CHECK(hipInit(0));
  MPI_Init(&argc,&argv);
  int rank = MPI_PROC_NULL;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);

  Cyclotron cyc;

  int usage = 0;
  if (rank == 0) {
    const char *const cmd = argv[0];
    for (char c = 0; c != -1; c = getopt(argc,argv,":hbi:r:s:")) {
      switch(c) {
        case '?':
          fprintf(stderr,"ERROR: %s unknown argument '-%c'\n",cmd,optopt);
          usage++;
          break;
        case ':':
          fprintf(stderr,"ERROR: %s missing argument for '-%c'\n",cmd,optopt);
          usage++;
          break;
        case 'h':
          usage++;
          break;
        case 'b':
          cyc.barrier = true;
          break;
        case 'i':
          if (sscanf(optarg,"%d",&cyc.iters) != 1) {
            fprintf(stderr,"ERROR: %s bad argument '-i %s'\n",cmd,optarg);
            usage++;
          }
          break;
        case 'r':
          if (!cyc.ropt.set(optarg)) {
            fprintf(stderr,"ERROR: %s bad argument '-r %s'\n",cmd,optarg);
            usage++;
          }
          break;
        case 's':
          if (!cyc.sopt.set(optarg)) {
            fprintf(stderr,"ERROR: %s bad argument '-s %s'\n",cmd,optarg);
            usage++;
          }
          break;
      }
    }

    if (usage) {
      fprintf(stderr,"Usage: %s [-h] | [-b] [-i <number>] [-r <loc>:<offset>:<delta>:<extra>] [-s <loc>:<offset>:<delta>:<extra>]\n",cmd);
      fprintf(stderr,"\t-h\t print this usage message and exit\n");
      fprintf(stderr,"\t-b\t add MPI_Barrier before each MPI_Allreduce\n");
      fprintf(stderr,"\t-i\t number of iterations\n");
      fprintf(stderr,"\t-r\t recv options\n");
      fprintf(stderr,"\t-s\t send options\n");
      fprintf(stderr,"\t<loc> can be 'host' or 'gpu' or 'stack' (only the first letter matters)\n");
      fprintf(stderr,"\t<offset> is the offset from the base allocation pointer, in doubles\n");
      fprintf(stderr,"\t<delta> is the additional offset per iteration, in doubles\n");
      fprintf(stderr,"\t<extra> is extra allocation after offset and buffer, in doubles\n");
      fprintf(stderr,"\t<offset>, <delta>, and <extra> are ignored for 'stack'\n");
      fflush(stderr);
    }
  }
  MPI_Bcast(&usage,1,MPI_INT,0,MPI_COMM_WORLD);
  if (usage) {
    MPI_Finalize();
    return usage;
  }

  MPI_Bcast(&cyc,sizeof(cyc),MPI_BYTE,0,MPI_COMM_WORLD);
  cyc.run();
  MPI_Finalize();
  return 0;
}
