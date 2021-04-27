# affinity-test

openmp:

- OMP_PROC_BIND
- OMP_PLACES

openmpi:

- --bind-to

Notes:

1. OpenMPI 在 npernode <=2 时默认是 --bind-to core，此时如果用多线程，记得要修改这个参数，否则一个进程只会绑定在一个 core 上（1-2 个 hwt）。
2. 默认情况下，`OMP_NUM_THREADS` 等于 cpu affinity 的个数。