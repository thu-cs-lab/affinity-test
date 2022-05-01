#!/bin/bash
set -x

export OMP_PROC_BIND=TRUE
export OMP_PLACES=cores
export OMP_NUM_THREADS=8

LOCAL_RANK=$OMPI_COMM_WORLD_LOCAL_RANK
socket=$(expr $LOCAL_RANK / 4)
cpu_lower=$(expr $LOCAL_RANK \* 16)
cpu_upper=$(expr $LOCAL_RANK \* 16 + 7)

exec numactl -C "${cpu_lower}-${cpu_upper}" -m $socket ./affinity-test
