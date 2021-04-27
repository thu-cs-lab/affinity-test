#!/bin/sh
set -e
make
MPIRUN=$(which mpirun)
set -x
#export OMP_NUM_THREADS=1
#export OMP_PROC_BIND=true
#export OMP_PLACES=cores
./affinity-test
$MPIRUN -np 4 --bind-to core --report-bindings ./affinity-test
$MPIRUN -np 4 --bind-to socket --report-bindings ./affinity-test
$MPIRUN -np 4 --bind-to hwthread --report-bindings ./affinity-test
$MPIRUN -np 4 --bind-to l1cache --report-bindings ./affinity-test
$MPIRUN -np 4 --bind-to numa --report-bindings ./affinity-test
$MPIRUN -np 4 --bind-to board --report-bindings ./affinity-test