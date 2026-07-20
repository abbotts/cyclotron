#include <algorithm>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mpi.h>
#include <vector>

#include "Cyclotron.h"
#include "gpu.h"

bool BufferOptions::set(const char *arg)
{
  if (!arg) return false;

  if (*arg == 'g') {
    loc = Location::GPU;
  } else if (*arg == 'h') {
    loc = Location::HOST;
  } else if (*arg == 's') {
    loc = Location::STACK;
  } else {
    return false;
  }

  for (; *arg != ':'; arg++) if (*arg == 0) return true;

  long newOffset, newDelta, newExtra;
  if (sscanf(arg,":%ld:%ld:%ld",&newOffset,&newDelta,&newExtra) == 3) {
    offset = newOffset;
    delta = newDelta;
    extra = newExtra;
  } else if (sscanf(arg,":%ld:%ld",&newOffset,&newDelta) == 2) {
    offset = newOffset;
    delta = newDelta;
  } else if (sscanf(arg,":%ld",&newOffset) == 1) {
    offset = newOffset;
  } else {
    return false;
  }
  return true;
}

void Cyclotron::run() const
{
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
    if (sopt.loc == Location::STACK) {
      printf(" on the stack");
    } else {
      printf(" %s allocated",(sopt.loc == Location::GPU ? "GPU" : "host"));
      if (sopt.offset) printf(" offset %lu doubles from the base pointer",sopt.offset);
      if (sopt.delta) printf(" plus %lu doubles each iteration",sopt.delta);
      if (sopt.extra) printf(" with %lu doubles extra",sopt.extra);
    }
    printf(" and recv is");
    if (ropt.loc == Location::STACK) {
      printf(" on the stack");
    } else {
      printf(" %s allocated",(ropt.loc == Location::GPU ? "GPU" : "host"));
      if (ropt.offset) printf(" offset %lu doubles from the base pointer",ropt.offset);
      if (ropt.delta) printf(" plus %lu doubles each iteration",ropt.delta);
      if (ropt.extra) printf(" with %lu doubles extra",ropt.extra);
    }
    printf("\n");
    fflush(stdout);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  if (iters <= 0) return;

  const double sendValue = double(rank+1);
  const double expected = double(commSize)*double(commSize+1)/2.0;

  double *recvBase = nullptr;
  const long recvSize = sizeof(double)*(ropt.offset+1+(iters-1)*ropt.delta+ropt.extra);
  if (ropt.loc != Location::STACK) {
    if (ropt.loc == Location::GPU) {
      CHECK(hipMalloc(&recvBase,recvSize));
      CHECK(hipMemset(recvBase,0,recvSize));
      CHECK(hipDeviceSynchronize());
    } else { // Location::HOST
      recvBase = reinterpret_cast<double*>(malloc(recvSize));
      assert(recvBase);
      memset(recvBase,0,recvSize);
    }
  }

  double *sendBase = nullptr;
  const long sendCount = sopt.offset+1+(iters-1)*sopt.delta+sopt.extra;
  const long sendSize = sizeof(double)*sendCount;
  if (sopt.loc != Location::STACK) {
    std::vector<double> sendCopy(sendCount,sendValue);
    if (sopt.loc == Location::GPU) {
      CHECK(hipMalloc(&sendBase,sendSize));
      CHECK(hipMemcpyHtoD(sendBase,sendCopy.data(),sendSize));
      CHECK(hipDeviceSynchronize());
    } else { // Location::HOST
      sendBase = reinterpret_cast<double*>(malloc(sendSize));
      memcpy(sendBase,sendCopy.data(),sendSize);
    }
  }

  std::vector<double> times(iters);
  std::vector<double> results(iters);

  // run benchmark

  MPI_Barrier(MPI_COMM_WORLD);
  for (int i = 0; i < iters; i++) {
    double *recv = nullptr;
    double *send = nullptr;
    double recvTemp, sendTemp;
    if (ropt.loc == Location::STACK) {
      recvTemp = 0;
      recv = &recvTemp;
    } else {
      recv = recvBase+ropt.offset+i*ropt.delta;
    }
    if (sopt.loc == Location::STACK) {
      sendTemp = sendValue;
      send = &sendTemp;
    } else {
      send = sendBase+sopt.offset+i*sopt.delta;
    }
    const double before = MPI_Wtime();
    if (barrier) MPI_Barrier(MPI_COMM_WORLD);
    MPI_Allreduce(send,recv,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
    const double after = MPI_Wtime();
    times[i] = after-before;
    results[i] = *recv;
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // verify results

  const double maxDiff = expected*double(FLT_EPSILON);
  int errors = 0;
  for (int i = 0; i < iters; i++) {
    const double diff = std::abs(expected-results[i]);
    if (diff > maxDiff) {
      errors++;
      fprintf(stderr,"%s ERROR #%d at rank %d iteration %d: expected %g, got %g, diff %g > %g\n",__FUNCTION__,errors,rank,i,expected,results[i],diff,maxDiff);
      fflush(stderr);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (errors) exit(errors);

  // print results

  std::vector<double> maxTimes(iters), minTimes(iters);
  MPI_Allreduce(times.data(),maxTimes.data(),iters,MPI_DOUBLE,MPI_MAX,MPI_COMM_WORLD);
  MPI_Allreduce(times.data(),minTimes.data(),iters,MPI_DOUBLE,MPI_MIN,MPI_COMM_WORLD);

  std::vector<int> fastest(iters,MPI_PROC_NULL), slowest(iters,MPI_PROC_NULL);
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
    if (ropt.loc == Location::GPU) {
      CHECK(hipFree(recvBase));
    } else {
      free(recvBase);
    }
    recvBase = nullptr;
  }
  if (sendBase) {
    if (sopt.loc == Location::GPU) {
      CHECK(hipFree(sendBase));
    } else {
      free(sendBase);
    }
    sendBase = nullptr;
  }
}
