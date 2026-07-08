#!/bin/bash
module load cpe/26.03
module load rocm
module -t list

export LD_LIBRARY_PATH="${CRAY_LD_LIBRARY_PATH}:${LD_LIBRARY_PATH}"
set -x
#ldd cyclotron

export MPICH_GPU_SUPPORT_ENABLED=1
srun -p testing -t3 -N2 -n16 --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron

