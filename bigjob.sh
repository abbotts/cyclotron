#!/bin/bash
module load cpe/26.03
module load rocm
module -t list

export LD_LIBRARY_PATH="${CRAY_LD_LIBRARY_PATH}:${LD_LIBRARY_PATH}"
set -x
date

ldd cyclotron

TASKS=$(( 8 * SLURM_NNODES ))
export MPICH_OFI_CXI_COUNTER_REPORT=2
#export MPICH_COLL_SYNC_STATS=1
#export MPICH_CSEL_VERBOSE=1

export MPICH_ENV_DISPLAY=1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron
sleep 1

export MPICH_ENV_DISPLAY=0

srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -b
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -c
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -b -c
sleep 1

export MPICH_GPU_SUPPORT_ENABLED=1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -r gpu
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -s gpu
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -b
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -r gpu -b
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -s gpu -b
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -c
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -r gpu -c
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -s gpu -c
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -b -c
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -r gpu -b -c
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --gpus-per-task=1 -c1 --gpu-bind=closest ./cyclotron -s gpu -b -c
sleep 1

date
