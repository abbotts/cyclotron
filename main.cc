#include <cstdio>
#include <mpi.h>
#include <unistd.h>
#include <filesystem>
#include <cstring>
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
  
  int arglen = 0;
  const char *const cmd = argv[0];
  for (char c = 0; c != -1; c = getopt(argc,argv,":hbci:r:s:o:")) {
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
      case 'c':
        cyc.switcheroo = true;
        break;
      case 'o':
        fprintf(stderr, "option -o with argument '%s'\n", optarg);
        arglen = std::strlen(optarg);
        copy(optarg, optarg + arglen, std::back_inserter(cyc.output_path));
        if (cyc.output_path.empty()) {
          fprintf(stderr,"ERROR: %s bad argument '-o %s'\n",cmd,optarg);
          usage++;
        }
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

  if (usage && rank == 0) {
    fprintf(stderr,"Usage: %s [-h] | [-b] [-i <number>] [-r <loc>:<offset>:<delta>:<extra>] [-s <loc>:<offset>:<delta>:<extra>]\n",cmd);
    fprintf(stderr,"\t-h\t print this usage message and exit\n");
    fprintf(stderr,"\t-b\t add MPI_Barrier before each MPI_Allreduce\n");
    fprintf(stderr,"\t-c\t record nonvoluntary context switches\n");
    fprintf(stderr,"\t-i\t number of iterations\n");
    fprintf(stderr,"\t-r\t recv options\n");
    fprintf(stderr,"\t-s\t send options\n");
    fprintf(stderr,"\t<loc> can be 'host' or 'gpu' or 'stack' (only the first letter matters)\n");
    fprintf(stderr,"\t<offset> is the offset from the base allocation pointer, in doubles\n");
    fprintf(stderr,"\t<delta> is the additional offset per iteration, in doubles\n");
    fprintf(stderr,"\t<extra> is extra allocation after offset and buffer, in doubles\n");
    fprintf(stderr,"\t<offset>, <delta>, and <extra> are ignored for 'stack'\n");
    fprintf(stderr,"\t-o <output-path> turns on full detailed output and specifies the given path\n");
    fflush(stderr);
  }
  
  if (usage) {
    MPI_Finalize();
    return usage;
  }

  // Create the output directory, and warn if we're going to be clobbering it
  if (rank == 0) {
    if (!cyc.output_path.empty()) {
      fprintf(stderr, "Creating output directory '%s'\n", cyc.output_path.c_str());
      auto already_exists = std::filesystem::create_directory(cyc.output_path);
      if (!already_exists) {
        fprintf(stderr, "Warning: overwriting existing directory '%s'\n", cyc.output_path.c_str());
      }
    }
  }
  
  cyc.run();
  MPI_Finalize();
  return 0;
}
