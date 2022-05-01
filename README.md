# affinity-test

可视化进程绑定的小工具

## 如何使用

在有 OpenMP 和 MPI 的环境下，运行 `make` 即可得到 `affinity-test`。它会打印出每个进程和线程的核心绑定状态，并生成 `affinity.gnuplot` 可视化绑定关系。如果提供 `-p` 选项，则还会调用 gnuplot 生成 PNG。

备注：

1. OpenMPI 在 npernode <=2 时默认是 `--bind-to core`。如果用多线程，记得要修改这个参数，否则一个进程只会绑定在一个 core 上（1-2 个 hardware thread）。
2. 默认情况下，`OMP_NUM_THREADS` 等于 cpu affinity 的个数，MPI 环境下需要手工指定，避免 spawn 过多线程。
