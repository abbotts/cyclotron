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
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r stack
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s stack
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r stack -s stack
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:8
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:264
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:4104
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:4104:4104
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:8
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:264
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:4104
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:4104:4104
sleep 1

export MPICH_GPU_SUPPORT_ENABLED=1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r stack
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s stack
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r stack -s stack
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:8
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:264
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:4104
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r host:4104:4104
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:8
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:264
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:4104
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s host:4104:4104
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r gpu
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r gpu:8
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r gpu:264
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r gpu:4104
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -r gpu:4104:4104
sleep 1

srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s gpu
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s gpu:8
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s gpu:264
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s gpu:4104
sleep 1
srun -t3 -N${SLURM_NNODES} -n${TASKS} --ntasks-per-gpu=1 --gpu-bind=closest ./cyclotron -s gpu:4104:4104


