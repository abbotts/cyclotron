#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <mpi.h>
#include <vector>

#include "cyclotron.h"
#include "gpu.h"

void cyclotron(const int iters,
               const size_t recvExtra,
               const size_t recvOffset,
               const size_t sendExtra,
               const size_t sendOffset,
               const bool recvGPU,
               const bool sendGPU,
               const bool recvStack,
               const bool sendStack,
               const bool barrier)
{

  // init

  int rank = MPI_PROC_NULL;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  int commSize = 0;
  MPI_Comm_size(MPI_COMM_WORLD,&commSize);

  if (rank == 0) {
    printf("### %s: %d calls",__FUNCTION__,iters);
    printf(" to MPI_Allreduce(send,recv,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD)");
    printf(" on %d tasks", commSize);
    if (barrier) printf(" preceded by an MPI_Barrier(MPI_COMM_WORLD)");
    printf(" where send is");
    if (sendStack) {
      printf(" on the stack");
    } else {
      printf(" %s allocated",(sendGPU ? "GPU" : "host"));
      if (sendOffset) printf(" offset %luB from the base pointer",sendOffset);
      if (sendExtra) printf(" with %luB extra",sendExtra);
    }
    printf(" and recv is");
    if (recvStack) {
      printf(" on the stack");
    } else {
      printf(" %s allocated",(recvGPU ? "GPU" : "host"));
      if (recvOffset) printf(" offset %luB from the base pointer",recvOffset);
      if (recvExtra) printf(" with %luB extra",recvExtra);
    }
    printf("\n");
    fflush(stdout);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  if (iters <= 0) return;

  const double sendValue = double(rank+1);
  const double expected = double(commSize*(commSize+1)/2);

  double *recv = nullptr;
  char *recvBase = nullptr;
  const size_t recvSize = sizeof(double)+recvOffset+recvExtra;
  if (!recvStack) {
    if (recvGPU) {
      CHECK(hipMalloc(&recvBase,recvSize));
      CHECK(hipMemset(recvBase,0,recvSize));
      CHECK(hipDeviceSynchronize());
    } else {
      recvBase = reinterpret_cast<char*>(malloc(recvSize));
      assert(recvBase);
      memset(recvBase,0,recvSize);
    }
    recv = reinterpret_cast<double*>(recvBase+recvOffset);
  }

  double *send = nullptr;
  char *sendBase = nullptr;
  const size_t sendSize = sizeof(double)+sendOffset+sendExtra;
  if (!sendStack) {
    if (sendGPU) {
      CHECK(hipMalloc(&sendBase,sendSize));
      CHECK(hipMemset(sendBase,0,sendSize));
      CHECK(hipDeviceSynchronize());
    } else {
      sendBase = reinterpret_cast<char*>(malloc(sendSize));
      memset(sendBase,0,sendSize);
    }
    send = reinterpret_cast<double*>(sendBase+sendOffset);
    if (sendGPU) CHECK(hipMemcpyHtoD(send,&sendValue,sizeof(sendValue)));
    else *send = sendValue;
  }

  std::vector<double> times(iters);
  std::vector<double> results(iters);

  // run benchmark

  MPI_Barrier(MPI_COMM_WORLD);
  for (int i = 0; i < iters; i++) {
    double recvTemp, sendTemp;
    if (recvStack) {
      recvTemp = 0;
      recv = &recvTemp;
    }
    if (sendStack) {
      sendTemp = sendValue;
      send = &sendTemp;
    }
    const double before = MPI_Wtime();
    if (barrier) MPI_Barrier(MPI_COMM_WORLD);
    MPI_Allreduce(send,recv,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
    const double after = MPI_Wtime();
    times[i] = after-before;
    if (recvGPU) CHECK(hipMemcpyDtoH(results.data()+i,recv,sizeof(double)));
    else results[i] = *recv;
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // verify results

  const double maxDiff = expected*double(FLT_EPSILON);
  int errors = 0;
  for (int i = 0; i < iters; i++) {
    const double diff = std::abs(expected-results[i]);
    if (diff > maxDiff) {
      errors++;
      fprintf(stderr,"__FUNCTION__ ERROR #%d at rank %d iteration %d: expected %g, got %g, diff %g > %g\n",errors,rank,i,expected,results[i],diff,maxDiff);
      fflush(stderr);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (errors) exit(errors);

  // print results

  std::vector<double> maxTimes(iters), minTimes(iters);
  MPI_Allreduce(times.data(),maxTimes.data(),iters,MPI_DOUBLE,MPI_MAX,MPI_COMM_WORLD);
  MPI_Allreduce(times.data(),minTimes.data(),iters,MPI_DOUBLE,MPI_MIN,MPI_COMM_WORLD);

  std::vector<int> fastest(iters,-1), slowest(iters,-1);
  for (int i = 0; i < iters; i++) {
    if (times[i] == minTimes[i]) fastest[i] = rank;
    if (times[i] == maxTimes[i]) slowest[i] = rank;
  }

  std::vector<double> avgTimes;
  std::vector<int> maxFastest, maxSlowest;
  if (rank == 0) {
    avgTimes.resize(iters);
    maxFastest.resize(iters);
    maxSlowest.resize(iters);
  }
  MPI_Reduce(times.data(),avgTimes.data(),iters,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(fastest.data(),maxFastest.data(),iters,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);
  MPI_Reduce(slowest.data(),maxSlowest.data(),iters,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);

  if (rank == 0) {
    const double us = 1e6;
    const double usAvg = us/double(commSize);
    for (int i = 0; i < iters; i++) {
      maxTimes[i] *= us;
      minTimes[i] *= us;
      avgTimes[i] *= usAvg;
    }

    printf("# times for first iteration (us): min %g avg %g max %g\n",minTimes[0],avgTimes[0],maxTimes[0]);

    if (iters > 1) {
      double avgTime = avgTimes[1];
      double maxTime = maxTimes[1];
      double minTime = minTimes[1];
      for (int i = 2; i < iters; i++) {
        avgTime += avgTimes[i];
        maxTime = std::max(maxTime,maxTimes[i]);
        minTime = std::min(minTime,minTimes[i]);
      }
      avgTime /= double(iters-1);
      printf("# times for remaining %d iterations (us): min %g avg %g max %g\n",iters-1,minTime,avgTime,maxTime);
    }

    printf("\n\n# times and ranks: iteration | min (us) | rank of min | avg (us) | max (us) | rank of max\n");
    for (int i = 0; i < iters; i++) {
      printf("%d %g %d %g %g %d\n",i,minTimes[i],maxFastest[i],avgTimes[i],maxTimes[i],maxSlowest[i]);
    }

    if (iters > 1) {

      std::sort(avgTimes.begin()+1,avgTimes.end());
      std::sort(maxTimes.begin()+1,maxTimes.end());
      std::sort(minTimes.begin()+1,minTimes.end());

      std::reverse(avgTimes.begin()+1,avgTimes.end());
      std::reverse(maxTimes.begin()+1,maxTimes.end());
      std::reverse(minTimes.begin()+1,minTimes.end());

      printf("\n\n# sorted excluding first iteration: place | min (us) | avg(us) | max (us)\n");
      for (int i = 1; i < iters; i++) {
        printf("%d %g %g %g\n",i,minTimes[i],avgTimes[i],maxTimes[i]);
      }

      std::vector<int> rankFastest(commSize), rankSlowest(commSize);
      for (int i = 1; i < iters; i++) {
        rankFastest.at(maxFastest[i])++;
        rankSlowest.at(maxSlowest[i])++;
      }

      printf("\n\n# count excluding first iteration: rank | fastest | slowest\n");
      for (int i = 0; i < commSize; i++) {
        printf("%d %d %d\n",i,rankFastest[i],rankSlowest[i]);
      }
    }
    fflush(stdout);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // cleanup

  if (recvBase) {
    recv = nullptr;
    if (recvGPU) CHECK(hipFree(recvBase));
    else free(recvBase);
    recvBase = nullptr;
  }
  if (sendBase) {
    send = nullptr;
    if (sendGPU) CHECK(hipFree(sendBase));
    else free(sendBase);
    sendBase = nullptr;
  }
}
