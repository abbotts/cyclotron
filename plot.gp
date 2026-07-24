INPUT = ARGV[1]
OUTPUT = ARGV[2]
TASKSTRING = `grep TASKS @INPUT | sed 's/.*=//'`
TASKS = int(TASKSTRING)
NODES = int(TASKS/8)
DATA = OUTPUT."-".TASKSTRING.".dat"
!sed '/^[\/+A-Za-z[:blank:]][A-Za-z[:blank:]]*/d' @INPUT > @DATA

set colorsequence podo
set terminal pdf
set output OUTPUT."-".TASKSTRING.".pdf"

set logscale y
set yrange [10:30000]
set ylabel 'Time (us)'

set xrange [-0.01*TASKS : 1.01*TASKS ]
set xlabel 'MPI Rank'

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=0",TASKS,NODES)
plot DATA using 1:4 index 5 title 'Slowest' lc 1, DATA using 1:3 index 5 title 'Average' lc 2, DATA using 1:2 index 5 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=0",TASKS,NODES)
plot DATA using 1:4 index 8 title 'Slowest' lc 1, DATA using 1:3 index 8 title 'Average' lc 2, DATA using 1:2 index 8 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:4 index 17 title 'Slowest' lc 1, DATA using 1:3 index 17 title 'Average' lc 2, DATA using 1:2 index 17 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, GPU Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:4 index 20 title 'Slowest' lc 1, DATA using 1:3 index 20 title 'Average' lc 2, DATA using 1:2 index 20 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nGPU Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:4 index 23 title 'Slowest' lc 1, DATA using 1:3 index 23 title 'Average' lc 2, DATA using 1:2 index 23 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:4 index 26 title 'Slowest' lc 1, DATA using 1:3 index 26 title 'Average' lc 2, DATA using 1:2 index 26 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, GPU Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:4 index 29 title 'Slowest' lc 1, DATA using 1:3 index 29 title 'Average' lc 2, DATA using 1:2 index 29 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nGPU Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:4 index 32 title 'Slowest' lc 1, DATA using 1:3 index 32 title 'Average' lc 2, DATA using 1:2 index 32 title 'Fastest' lc 3

set xrange [-20:1020]
set xlabel 'Iteration'

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=0",TASKS,NODES)
plot DATA using 1:5 index 3 title 'Slowest' lc 1, DATA using 1:4 index 3 title 'Average' lc 2, DATA using 1:2 index 3 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=0",TASKS,NODES)
plot DATA using 1:5 index 6 title 'Slowest' lc 1, DATA using 1:4 index 6 title 'Average' lc 2, DATA using 1:2 index 6 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 15 title 'Slowest' lc 1, DATA using 1:4 index 15 title 'Average' lc 2, DATA using 1:2 index 15 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, GPU Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 18 title 'Slowest' lc 1, DATA using 1:4 index 18 title 'Average' lc 2, DATA using 1:2 index 18 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nGPU Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 21 title 'Slowest' lc 1, DATA using 1:4 index 21 title 'Average' lc 2, DATA using 1:2 index 21 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 24 title 'Slowest' lc 1, DATA using 1:4 index 24 title 'Average' lc 2, DATA using 1:2 index 24 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, GPU Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 27 title 'Slowest' lc 1, DATA using 1:4 index 27 title 'Average' lc 2, DATA using 1:2 index 27 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nGPU Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 30 title 'Slowest' lc 1, DATA using 1:4 index 30 title 'Average' lc 2, DATA using 1:2 index 30 title 'Fastest' lc 3

unset log y2
set y2label "Nonvoluntary Context Switches"
set y2range [0:15]
set y2tics

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=0",TASKS,NODES)
plot DATA using 1:9 index 9 notitle axes x1y2 lc rgb 'gray', DATA using 1:5 index 9 title 'Slowest' lc 1, DATA using 1:4 index 9 title 'Average' lc 2, DATA using 1:2 index 9 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=0",TASKS,NODES)
plot DATA using 1:9 index 12 notitle axes x1y2 lc rgb 'gray', DATA using 1:5 index 12 title 'Slowest' lc 1, DATA using 1:4 index 12 title 'Average' lc 2, DATA using 1:2 index 12 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:9 index 33 notitle axes x1y2 lc rgb 'gray', DATA using 1:5 index 33 title 'Slowest' lc 1, DATA using 1:4 index 33 title 'Average' lc 2, DATA using 1:2 index 33 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, GPU Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:9 index 36 notitle axes x1y2 lc rgb 'gray', DATA using 1:5 index 36 title 'Slowest' lc 1, DATA using 1:4 index 36 title 'Average' lc 2, DATA using 1:2 index 36 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nGPU Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:9 index 39 notitle axes x1y2 lc rgb 'gray', DATA using 1:5 index 39 title 'Slowest' lc 1, DATA using 1:4 index 39 title 'Average' lc 2, DATA using 1:2 index 39 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:9 index 42 notitle axes x1y2 lc rgb 'gray', DATA using 1:5 index 42 title 'Slowest' lc 1, DATA using 1:4 index 42 title 'Average' lc 2, DATA using 1:2 index 42 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, GPU Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:9 index 45 notitle axes x1y2 lc rgb 'gray', DATA using 1:5 index 45 title 'Slowest' lc 1, DATA using 1:4 index 45 title 'Average' lc 2, DATA using 1:2 index 45 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nGPU Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:9 index 48 notitle axes x1y2 lc rgb 'gray', DATA using 1:5 index 48 title 'Slowest' lc 1, DATA using 1:4 index 48 title 'Average' lc 2, DATA using 1:2 index 48 title 'Fastest' lc 3

set xrange [-0.01*TASKS : 1.01*TASKS ]
set xlabel 'MPI Rank'

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=0",TASKS,NODES)
plot DATA using 1:5 index 11 notitle axes x1y2 lc rgb 'gray', DATA using 1:4 index 11 title 'Slowest' lc 1, DATA using 1:3 index 11 title 'Average' lc 2, DATA using 1:2 index 11 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=0",TASKS,NODES)
plot DATA using 1:5 index 14 notitle axes x1y2 lc rgb 'gray', DATA using 1:4 index 14 title 'Slowest' lc 1, DATA using 1:3 index 14 title 'Average' lc 2, DATA using 1:2 index 14 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=0",TASKS,NODES)
plot DATA using 1:5 index 35 notitle axes x1y2 lc rgb 'gray', DATA using 1:4 index 35 title 'Slowest' lc 1, DATA using 1:3 index 35 title 'Average' lc 2, DATA using 1:2 index 35 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nHost Send Buffer, GPU Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 38 notitle axes x1y2 lc rgb 'gray', DATA using 1:4 index 38 title 'Slowest' lc 1, DATA using 1:3 index 38 title 'Average' lc 2, DATA using 1:2 index 38 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM on %d Tasks (%d Nodes)\nGPU Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 41 notitle axes x1y2 lc rgb 'gray', DATA using 1:4 index 41 title 'Slowest' lc 1, DATA using 1:3 index 41 title 'Average' lc 2, DATA using 1:2 index 41 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 44 notitle axes x1y2 lc rgb 'gray', DATA using 1:4 index 44 title 'Slowest' lc 1, DATA using 1:3 index 44 title 'Average' lc 2, DATA using 1:2 index 44 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nHost Send Buffer, GPU Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 47 notitle axes x1y2 lc rgb 'gray', DATA using 1:4 index 47 title 'Slowest' lc 1, DATA using 1:3 index 47 title 'Average' lc 2, DATA using 1:2 index 47 title 'Fastest' lc 3

set title sprintf("Single MPI\\_DOUBLE MPI\\_SUM after MPI\\_Barrier on %d Tasks (%d Nodes)\nGPU Send Buffer, Host Recv Buffer, MPICH\\_GPU\\_SUPPORT\\_ENABLED=1",TASKS,NODES)
plot DATA using 1:5 index 50 notitle axes x1y2 lc rgb 'gray', DATA using 1:4 index 50 title 'Slowest' lc 1, DATA using 1:3 index 50 title 'Average' lc 2, DATA using 1:2 index 50 title 'Fastest' lc 3


