#include <algorithm>
#include <atomic>
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mpi.h>
#include <thread>
#include <vector>
#include <chrono>

#include "Cyclotron.h"
#include "gpu.h"
#include "bits.h"

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

static int switchInits = 0;

static long readSwitches()
{
  static char *line = nullptr;
  static size_t cap = 0;
  static FILE *file = nullptr;

  if (!file) {
      file = fopen("/proc/self/status","r");
      assert(file);
      switchInits++;
      assert(switchInits == 1);
  }

  long switches = 0;
  rewind(file);
  while (getline(&line,&cap,file) != -1) {
    if (sscanf(line,"nonvoluntary_ctxt_switches: %ld",&switches) == 1) {
      return switches;
    }
  }
  return -1;
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
    if (barrier) printf(" preceded by an untimed MPI_Barrier(MPI_COMM_WORLD)");
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
    if (switcheroo) printf(" with counts of nonvoluntary context switches");
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

  // Record the start time of the benchmark using high-resolution clock
  // Let rank 0 record the start time and broadcast it to all other ranks
  std::chrono::high_resolution_clock::time_point begin;
  if (rank == 0) {
    begin = std::chrono::high_resolution_clock::now();
  }

  MPI_Bcast(&begin, sizeof(begin), MPI_BYTE, 0, MPI_COMM_WORLD);


  std::vector<double[3]> times(iters);
  std::vector<double> results(iters);

  FILE *procStatus = nullptr;
  std::vector<long> switches;
  if (switcheroo) {
    switches.resize(iters,0);
    readSwitches();
  }

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
    if (barrier) MPI_Barrier(MPI_COMM_WORLD);
    size_t switchesBefore;
    if (switcheroo) switchesBefore = readSwitches();
    const auto before = std::chrono::high_resolution_clock::now();
    MPI_Allreduce(send,recv,1,MPI_DOUBLE,MPI_SUM,MPI_COMM_WORLD);
    const auto after = std::chrono::high_resolution_clock::now();
    if (switcheroo) switches[i] = readSwitches()-switchesBefore;
    // store the elapsed time in the first element of the array to minimally disrupt the past
    // need to convert from std::chrono::high_resolution_clock::duration to double representing s (which MPI_WTime would have returned)
    times[i][0] = std::chrono::duration<double>(after-before).count();
    // Store realtimes since benchmark start for before and after
    times[i][1] = std::chrono::duration<double>(before-begin).count();
    times[i][2] = std::chrono::duration<double>(after-begin).count();
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
      Mfprintf(stderr,"%s ERROR #%d at rank %d iteration %d: expected %g, got %g, diff %g > %g\n",__FUNCTION__,errors,rank,i,expected,results[i],diff,maxDiff);
      fflush(stderr);
    }
  }
  MPI_Barrier(MPI_COMM_WORLD);
  if (errors) exit(errors);

  const double us = 1e6;
  
  // If we have an output path, write the results to a file
  // We'll write one file per data point: times, context switches
  if (!output_path.empty()) {
    // FIXME - After the current investigations we should move all this to a dedicated function,
    // and use different data output formats. Having this be a multi-dimensional hdf5 file dataset would be preferred.
    std::string times_path = output_path + "/times.asc";
    std::string header = "# Iteration times (us)\n";
    std::string data_string = "";
    for (int i = 0; i < iters; i++) {
      // Yes this is slow as molasses. As long as iter_count isn't gigantic it should be fine
      if (i > 0) data_string += " ";
      data_string += std::to_string(times[i] * us);
    }
    data_string += "\n";
    
    if (rank == 0) {
      data_string = header + data_string;
    }

    MPI_File fh;
    MPI_File_open(MPI_COMM_WORLD,times_path.c_str(),MPI_MODE_CREATE|MPI_MODE_WRONLY,MPI_INFO_NULL,&fh);
    MPI_File_write_ordered(fh,data_string.c_str(),data_string.size(),MPI_CHAR,MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    // I know it seems silly, but the easiest way for me to handle start and end time data of each collective
    // is if we just write starts to one file and ends to the other
    std::string starts_path = output_path + "/starts.asc";
    std::string ends_path = output_path + "/ends.asc";
    header = "# rank | Iteration start times (us)\n";
    data_string = std::to_string(rank);
    for (int i = 0; i < iters; i++) {
      data_string += " " + std::to_string(times[i][1] * us);
    }
    data_string += "\n";

    if (rank == 0) {
      data_string = header + data_string;
    }
    MPI_File_open(MPI_COMM_WORLD,starts_path.c_str(),MPI_MODE_CREATE|MPI_MODE_WRONLY,MPI_INFO_NULL,&fh);
    MPI_File_write_ordered(fh,data_string.c_str(),data_string.size(),MPI_CHAR,MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    header = "# rank | Iteration end times (us)\n";
    data_string = std::to_string(rank);
    for (int i = 0; i < iters; i++) {
      data_string += " " + std::to_string(times[i][2] * us);
    }
    data_string += "\n";

    if (rank == 0) {
      data_string = header + data_string;
    }
    MPI_File_open(MPI_COMM_WORLD,ends_path.c_str(),MPI_MODE_CREATE|MPI_MODE_WRONLY,MPI_INFO_NULL,&fh);
    MPI_File_write_ordered(fh,data_string.c_str(),data_string.size(),MPI_CHAR,MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    if (switcheroo) {
      std::string switches_path = output_path + "/switches.asc";
      header = "# Iteration context switches\n";
      data_string = "";
      for (int i = 0; i < iters; i++) {
        if (i > 0) data_string += " ";
        data_string += std::to_string(switches[i]);
      }
      data_string += "\n";

      if (rank == 0) {
        data_string = header + data_string;
      }
      MPI_File_open(MPI_COMM_WORLD,switches_path.c_str(),MPI_MODE_CREATE|MPI_MODE_WRONLY,MPI_INFO_NULL,&fh);
      MPI_File_write_ordered(fh,data_string.c_str(),data_string.size(),MPI_CHAR,MPI_STATUS_IGNORE);
      MPI_File_close(&fh);
    }

    MPI_Barrier(MPI_COMM_WORLD);
  
  }

  // print results

  std::vector<double> elapsedTimes(iters);
  for (int i = 0; i < iters; i++) elapsedTimes[i] = times[i][0] * us;

  std::vector<double> maxTimes(iters), minTimes(iters);
  MPI_Allreduce(elapsedTimes.data(),maxTimes.data(),iters,MPI_DOUBLE,MPI_MAX,MPI_COMM_WORLD);
  MPI_Allreduce(elapsedTimes.data(),minTimes.data(),iters,MPI_DOUBLE,MPI_MIN,MPI_COMM_WORLD);

  std::vector<int> fastest(iters,MPI_PROC_NULL), slowest(iters,MPI_PROC_NULL);
  for (int i = 0; i < iters; i++) {
    if (elapsedTimes[i] == minTimes[i]) fastest[i] = rank;
    if (elapsedTimes[i] == maxTimes[i]) slowest[i] = rank;
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
  MPI_Reduce(elapsedTimes.data(),avgTimes.data(),iters,MPI_DOUBLE,MPI_SUM,0,MPI_COMM_WORLD);
  MPI_Reduce(fastest.data(),maxFastest.data(),iters,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);
  MPI_Reduce(slowest.data(),maxSlowest.data(),iters,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);


  if (iters > 1) {
    double avgTimeMe = elapsedTimes[1];
    double maxTimeMe = elapsedTimes[1];
    double minTimeMe = elapsedTimes[1];
    for (int i = 2; i < iters; i++) {
      avgTimeMe += elapsedTimes[i];
      maxTimeMe = std::max(maxTimeMe,elapsedTimes[i]);
      minTimeMe = std::min(minTimeMe,elapsedTimes[i]);
    }
    avgTimeMe /= double(iters-1);

    MPI_Gather(&avgTimeMe,1,MPI_DOUBLE,avgTimeTask.data(),1,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Gather(&maxTimeMe,1,MPI_DOUBLE,maxTimeTask.data(),1,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Gather(&minTimeMe,1,MPI_DOUBLE,minTimeTask.data(),1,MPI_DOUBLE,0,MPI_COMM_WORLD);
  }

  std::vector<long> maxSwitches,minSwitches,sumSwitches;
  std::vector<int> maxMaxSwitched,maxMinSwitched;
  std::vector<long> maxSwitchTask,minSwitchTask,sumSwitchTask;
  if (switcheroo) {
    maxSwitches.resize(iters);
    minSwitches.resize(iters);
    MPI_Allreduce(switches.data(),maxSwitches.data(),iters,MPI_LONG,MPI_MAX,MPI_COMM_WORLD);
    MPI_Allreduce(switches.data(),minSwitches.data(),iters,MPI_LONG,MPI_MIN,MPI_COMM_WORLD);
    std::vector<int> maxSwitched(iters), minSwitched(iters);
    for (int i = 0; i < iters; i++) {
      if (switches[i] == minSwitches[i]) minSwitched[i] = rank;
      if (switches[i] == maxSwitches[i]) maxSwitched[i] = rank;
    }
    if (rank == 0) {
      sumSwitches.resize(iters);
      maxMaxSwitched.resize(iters);
      maxMinSwitched.resize(iters);
    }
    MPI_Reduce(maxSwitched.data(),maxMaxSwitched.data(),iters,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(minSwitched.data(),maxMinSwitched.data(),iters,MPI_INT,MPI_MAX,0,MPI_COMM_WORLD);
    MPI_Reduce(switches.data(),sumSwitches.data(),iters,MPI_LONG,MPI_SUM,0,MPI_COMM_WORLD);

    if (iters > 0) {
      long maxSwitchMe = switches[1];
      long minSwitchMe = switches[1];
      long sumSwitchMe = switches[1];
      for (int i = 2; i < iters; i++) {
        maxSwitchMe = std::max(maxSwitchMe,switches[i]);
        minSwitchMe = std::min(minSwitchMe,switches[i]);
        sumSwitchMe += switches[i];
      }
      if (rank == 0) {
        maxSwitchTask.resize(commSize);
        minSwitchTask.resize(commSize);
        sumSwitchTask.resize(commSize);
      }
      MPI_Gather(&maxSwitchMe,1,MPI_LONG,maxSwitchTask.data(),1,MPI_LONG,0,MPI_COMM_WORLD);
      MPI_Gather(&minSwitchMe,1,MPI_LONG,minSwitchTask.data(),1,MPI_LONG,0,MPI_COMM_WORLD);
      MPI_Gather(&sumSwitchMe,1,MPI_LONG,sumSwitchTask.data(),1,MPI_LONG,0,MPI_COMM_WORLD);
    }
  }

  if (rank == 0) {
    const double perTask = 1.0/double(commSize);
    const double perIter = 1.0/double(iters-1);

    for (int i = 0; i < iters; i++) {
      avgTimes[i] *= perTask;
    }

    printf("## first iteration times (us): min %g avg %g max %g\n",minTimes[0],avgTimes[0],maxTimes[0]);
    if (switcheroo) printf("## first iteration nonvoluntary context switches: min %ld avg %g max %ld\n",minSwitches[0],double(sumSwitches[0])*perTask,maxSwitches[0]);

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

      if (switcheroo) {
        long minTask = minSwitches[1];
        long maxTask = maxSwitches[1];
        long minIter = sumSwitches[1];
        long sumIter = sumSwitches[1];
        long maxIter = sumSwitches[1];

        for (int i = 2; i < iters; i++) {
          minTask = std::min(minTask,minSwitches[i]);
          maxTask = std::max(maxTask,maxSwitches[i]);
          minIter = std::min(minIter,sumSwitches[i]);
          sumIter += sumSwitches[i];
          maxIter = std::max(maxIter,sumSwitches[i]);
        }
     
        const double avgIter = double(sumIter)*perIter;
        const double avgTask = avgIter*perTask;
        printf("## nonvoluntary context switches for remaining %d iterations: total %ld min/task %ld avg/task %g max/task %ld min/iter %ld avg/iter %g max/iter %ld\n",iters-1,sumIter,minTask,avgTask,maxTask,minIter,avgIter,maxIter);

      }
    }

    printf("\n\n# times and ranks: iteration | min (us) | rank of min | avg (us) | max (us) | rank of max");
    if (switcheroo) printf(" | min switches | rank of min | total switches | max swtiches | rank of max");
    printf("\n");
    for (int i = 0; i < iters; i++) {
      printf("%d %g %d %g %g %d",i,minTimes[i],maxFastest[i],avgTimes[i],maxTimes[i],maxSlowest[i]);
      if (switcheroo) printf(" %ld %d %ld %ld %d",minSwitches[i],maxMinSwitched[i],sumSwitches[i],maxSwitches[i],maxMaxSwitched[i]);
      printf("\n");
    }

    if (iters > 1) {

      std::sort(avgTimes.begin()+1,avgTimes.end());
      std::reverse(avgTimes.begin()+1,avgTimes.end());

      std::sort(maxTimes.begin()+1,maxTimes.end());
      std::reverse(maxTimes.begin()+1,maxTimes.end());

      std::sort(minTimes.begin()+1,minTimes.end());
      std::reverse(minTimes.begin()+1,minTimes.end());

      printf("\n\n# sorted excluding first iteration: place | min (us) | avg (us) | max (us)");
      if (switcheroo) {
        std::sort(sumSwitches.begin()+1,sumSwitches.end());
        std::reverse(sumSwitches.begin()+1,sumSwitches.end());

        std::sort(maxSwitches.begin()+1,maxSwitches.end());
        std::reverse(maxSwitches.begin()+1,maxSwitches.end());

        printf(" | total switches | max switches/task");
      }

      printf("\n");
      for (int i = 1; i < iters; i++) {
        printf("%d %g %g %g",i,minTimes[i],avgTimes[i],maxTimes[i]);
        if (switcheroo) printf(" %ld %ld",sumSwitches[i],maxSwitches[i]);
        printf("\n");
      }
    }

    printf("\n\n# rank | min (us) | avg (us) | max (us)");
    if (switcheroo) printf(" | total switches | min switches/iter | avg switches/iter | max switches/iter");
    printf("\n");
    for (int i = 0; i < commSize; i++) {
      printf("%d %g %g %g",i,minTimeTask[i],avgTimeTask[i],maxTimeTask[i]);
      if (switcheroo) {
        const double avgSwitchTask = double(sumSwitchTask[i])*perIter;
        printf(" %ld %ld %g %ld",sumSwitchTask[i],minSwitchTask[i],avgSwitchTask,maxSwitchTask[i]);
      }
      printf("\n");
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
