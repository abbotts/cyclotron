#include <algorithm>
#include <cfloat>
#include <cstdio>
#include <cstdlib>
#include <mpi.h>
#include <vector>

#include "Synchrotron.h"

void Synchrotron::run() const
{
  int rank = MPI_PROC_NULL;
  MPI_Comm_rank(MPI_COMM_WORLD,&rank);
  int commSize = 0;
  MPI_Comm_size(MPI_COMM_WORLD,&commSize);

  if (rank == 0) {
    printf("### %s: %d calls",__PRETTY_FUNCTION__,iters);
    printf(" to MPI_Allreduce(send,recv,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD)");
    printf(" on %d tasks", commSize);
    if (barrier) printf(" preceded by an untimed MPI_Barrier(MPI_COMM_WORLD)");
    printf("\n");
    fflush(stdout);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  if (iters <= 0) return;

  const double send = double(rank+1);
  const double expected = double(commSize)*double(commSize+1)/2.0;
  double recv = 0;

  std::vector<double> times(iters);
  std::vector<double> results(iters);

  // run benchmark

  MPI_Barrier(MPI_COMM_WORLD);
  for (int i = 0; i < iters; i++) {
    if (barrier) MPI_Barrier(MPI_COMM_WORLD);
    const double before = MPI_Wtime();
    MPI_Allreduce(&send,&recv,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
    const double after = MPI_Wtime();
    times[i] = after-before;
    results[i] = recv;
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

  const double us = 1e6;
  for (int i = 0; i < iters; i++) times[i] *= us;

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
  std::vector<double> avgTimeTask,maxTimeTask,minTimeTask;

  if (rank == 0) {
    avgTimes.resize(iters);
    maxFastest.resize(iters);
    maxSlowest.resize(iters);
    if (iters > 1) {
      avgTimeTask.resize(commSize);
      maxTimeTask.resize(commSize);
      minTimeTask.resize(commSize);
    }
  }
  MPI_Reduce(times.data(),avgTimes.data(),iters,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(fastest.data(),maxFastest.data(),iters,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);
  MPI_Reduce(slowest.data(),maxSlowest.data(),iters,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);


  if (iters > 1) {
    double avgTimeMe = times[1];
    double maxTimeMe = times[1];
    double minTimeMe = times[1];
    for (int i = 2; i < iters; i++) {
      avgTimeMe += times[i];
      maxTimeMe = std::max(maxTimeMe,times[i]);
      minTimeMe = std::min(minTimeMe,times[i]);
    }
    avgTimeMe /= double(iters-1);

    MPI_Gather(&avgTimeMe,1,MPI_DOUBLE,avgTimeTask.data(),1,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Gather(&maxTimeMe,1,MPI_DOUBLE,maxTimeTask.data(),1,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Gather(&minTimeMe,1,MPI_DOUBLE,minTimeTask.data(),1,MPI_DOUBLE,0,MPI_COMM_WORLD);
  }

  if (rank == 0) {
    const double perTask = 1.0/double(commSize);
    const double perIter = 1.0/double(iters-1);

    for (int i = 0; i < iters; i++) {
      avgTimes[i] *= perTask;
    }

    printf("## first iteration times (us): min %g avg %g max %g\n",minTimes[0],avgTimes[0],maxTimes[0]);

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
      printf("## times for remaining %d iterations (us): min %g avg %g max %g\n",iters-1,minTime,avgTime,maxTime);
    }

    printf("\n\n# times and ranks: iteration | min (us) | rank of min | avg (us) | max (us) | rank of max\n");
    for (int i = 0; i < iters; i++) {
      printf("%d %g %d %g %g %d\n",i,minTimes[i],maxFastest[i],avgTimes[i],maxTimes[i],maxSlowest[i]);
    }

    if (iters > 1) {

      std::sort(avgTimes.begin()+1,avgTimes.end());
      std::reverse(avgTimes.begin()+1,avgTimes.end());

      std::sort(maxTimes.begin()+1,maxTimes.end());
      std::reverse(maxTimes.begin()+1,maxTimes.end());

      std::sort(minTimes.begin()+1,minTimes.end());
      std::reverse(minTimes.begin()+1,minTimes.end());

      printf("\n\n# sorted excluding first iteration: place | min (us) | avg (us) | max (us)\n");
      for (int i = 1; i < iters; i++) {
        printf("%d %g %g %g\n",i,minTimes[i],avgTimes[i],maxTimes[i]);
      }
    }

    printf("\n\n# rank | min (us) | avg (us) | max (us)\n");
    for (int i = 0; i < commSize; i++) {
      printf("%d %g %g %g\n",i,minTimeTask[i],avgTimeTask[i],maxTimeTask[i]);
    }
    fflush(stdout);
  }
  MPI_Barrier(MPI_COMM_WORLD);
}
