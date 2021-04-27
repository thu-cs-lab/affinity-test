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
  int num_threads = omp_get_max_threads();
  cpu_set_t *all_masks = new cpu_set_t[num_threads + 1];

#pragma omp parallel for schedule(static, 1)
  for (int i = 0; i < num_threads; i++) {
    CPU_ZERO(&all_masks[i + 1]);
    assert(pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t),
                                  &all_masks[i + 1]) == 0);
  }

  // get process affinity
  CPU_ZERO(&all_masks[0]);
  assert(sched_getaffinity(0, sizeof(cpu_set_t), &all_masks[0]) == 0);

  cpu_set_t *root_buffer = NULL;
  if (rank == 0) {
    root_buffer = new cpu_set_t[(num_threads + 1) * size];
    memset(root_buffer, 0, sizeof(cpu_set_t) * (num_threads + 1) * size);
  }
  MPI_Gather(all_masks, sizeof(cpu_set_t) * (num_threads + 1), MPI_CHAR,
             root_buffer, sizeof(cpu_set_t) * (num_threads + 1), MPI_CHAR, 0,
             MPI_COMM_WORLD);
  if (rank == 0) {
    char buffer[1024];
    for (int i = 0; i < size; i++) {
      buffer[0] = 0;
      affinity_to_string(&root_buffer[i * (num_threads + 1)], buffer);
      printf("Rank %d Process: %s\n", i, buffer);
      for (int j = 1; j <= num_threads; j++) {
        buffer[0] = 0;
        affinity_to_string(&root_buffer[i * (num_threads + 1) + j], buffer);
        printf("Rank %d Thread %d: %s\n", i, j, buffer);
      }
    }
    delete[] root_buffer;
  }
  delete[] all_masks;

  MPI_Finalize();
  return 0;
}