#!/bin/bash
module load cpe/26.03
module load rocm
module -t list

export LD_LIBRARY_PATH="${CRAY_LD_LIBRARY_PATH}:${LD_LIBRARY_PATH}"
set -x
ldd cyclotron

TASKS=$(( 8 * SLURM_NNODES ))

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:0:1
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:0:1
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -b
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:0:1 -b
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:0:1 -b
sleep 1

export MPICH_GPU_SUPPORT_ENABLED=1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:0:1
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:0:1
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -b
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:0:1 -b
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:0:1 -b
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r gpu
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r gpu:0:1
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s gpu
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s gpu:0:1
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r gpu -b
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r gpu:0:1 -b
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s gpu -b
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s gpu:0:1 -b
sleep 1

