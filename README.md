# affinity-test

可视化绑核的小工具

## 如何使用

在一个有 OpenMP 和 MPI 的环境下，用 `make`，然后运行编译出来的程序即可。它会打印出每个进程和线程的核心绑定状态，并且使用 `gnuplot` 工具绘制出绑核结果。

## 绑核相关的参数和环境变量

openmp:

- OMP_PROC_BIND
- OMP_PLACES

srun:

- --cpu-bind

openmpi:

- --bind-to

备注：

1. OpenMPI 在 npernode <=2 时默认是 --bind-to core，此时如果用多线程，记得要修改这个参数，否则一个进程只会绑定在一个 core 上（1-2 个 hwt）。
2. 默认情况下，`OMP_NUM_THREADS` 等于 cpu affinity 的个数。
