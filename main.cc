#include <mpi.h>

#include "cyclotron.h"
#include "gpu.h"

int main(int argc, char **argv)
{
 CHECK(hipInit(0));
 MPI_Init(&argc,&argv);
 cyclotron();
 MPI_Finalize();
 return 0;
}
