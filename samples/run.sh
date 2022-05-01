#!/bin/sh
set -e
make
MPIRUN=$(which mpirun)
set -x
$MPIRUN -n 8 --bind-to none ./run_inner.sh
