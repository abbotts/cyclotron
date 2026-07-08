#!/bin/bash
module load cpe/26.03
module load rocm
module -t list

export LD_LIBRARY_PATH="${CRAY_LD_LIBRARY_PATH}:${LD_LIBRARY_PATH}"
set -x
make -j
