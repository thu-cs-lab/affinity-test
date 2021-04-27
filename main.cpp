#include <assert.h>
#include <mpi.h>
#include <omp.h>
#include <sched.h>
#include <stdio.h>
#include <string>
#include <sys/sysinfo.h>

void affinity_to_string(cpu_set_t *mask, char *buffer) {
  int nproc = get_nprocs();
  int last_zero = -1;
  for (int i = 0; i <= nproc; i++) {
    if (i == nproc || !CPU_ISSET(i, mask)) {
      if (i > last_zero + 1) {
        // stripe found
        if (i > last_zero + 2) {
          // more than one
          sprintf(buffer + strlen(buffer), " %d-%d", last_zero + 1, i - 1);
        } else {
          // only one
          sprintf(buffer + strlen(buffer), " %d", last_zero + 1);
        }
      }
      last_zero = i;
    }
  }
}

int main(int argc, char *argv[]) {
  MPI_Init(&argc, &argv);

  int size, rank;
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == 0) {
    printf("MPI Size: %d\n", size);
  }
  MPI_Barrier(MPI_COMM_WORLD);

  // get thread affinity
  cpu_set_t *thread_masks = new cpu_set_t[omp_get_max_threads()];
  if (rank == 0) {
#pragma omp parallel for schedule(static, 1)
    for (int i = 0; i < omp_get_max_threads(); i++) {
      CPU_ZERO(&thread_masks[i]);
      assert(pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t),
                                    &thread_masks[i]) == 0);
    }
  }

  // get process affinity
  cpu_set_t process_mask;
  CPU_ZERO(&process_mask);
  assert(sched_getaffinity(0, sizeof(cpu_set_t), &process_mask) == 0);

  // generate description based on affinity
  char buffer[1024] = {0};
  affinity_to_string(&mask, buffer);

  char *root_buffer = NULL;
  if (rank == 0) {
    root_buffer = new char[sizeof(buffer) * size];
  }
  MPI_Gather(buffer, sizeof(buffer), MPI_CHAR, root_buffer, sizeof(buffer),
             MPI_CHAR, 0, MPI_COMM_WORLD);
  if (rank == 0) {
    for (int i = 0; i < size; i++) {
      printf("Rank %d:%s\n", i, &root_buffer[i * sizeof(buffer)]);
    }
    delete[] root_buffer;
  }

  MPI_Finalize();
  return 0;
}