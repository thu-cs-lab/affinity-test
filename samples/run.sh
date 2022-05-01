#!/bin/sh
set -e

MPIRUN=$(which mpirun)

make

set -x
$MPIRUN -n 8 --bind-to none ./samples/run_inner.sh
