#include <string>
#include <cassert>
#include <cstring>
#include <cstdio>

#include <sched.h>
#include <sys/sysinfo.h>

#include <mpi.h>
#include <omp.h>

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

  bool call_gnuplot = false;
  if (argc > 1 && strcmp(argv[1], "-p") == 0) {
    call_gnuplot = true;
  };

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
    FILE *fp = fopen("affinity.gnuplot", "w");
    assert(fp);
    fprintf(fp, "set terminal png\n");
    fprintf(fp, "set title \"Thread affinity map\"\n");
    fprintf(fp, "unset key\n");
    fprintf(fp, "set palette rgbformula -7,2,-7\n");
    fprintf(fp, "unset colorbox\n");
    fprintf(fp, "$map << EOD\n");
    int nproc = get_nprocs();
    for (int i = 0; i < nproc; i++) {
      if (i % (nproc / 4) == 0) {
        fprintf(fp, ",%d", i);
      } else {
        fprintf(fp, ",");
      }
    }

    for (int i = 0; i < size; i++) {
      cpu_set_t *process_affinity = &root_buffer[i * (num_threads + 1)];

      buffer[0] = 0;
      affinity_to_string(process_affinity, buffer);
      printf("Rank %d Process: %s\n", i, buffer);

      for (int j = 1; j <= num_threads; j++) {
        cpu_set_t *thread_affinity = &root_buffer[i * (num_threads + 1) + j];
        if (j == 1) {
          fprintf(fp, "\nRank %d", i);
        } else {
          fprintf(fp, "\n");
        }
        for (int i = 0; i < nproc; i++) {
          fprintf(fp, ", %d", CPU_ISSET(i, thread_affinity) != 0);
        }

        buffer[0] = 0;
        affinity_to_string(&root_buffer[i * (num_threads + 1) + j], buffer);
        printf("Rank %d Thread %d: %s\n", i, j, buffer);
      }
    }

    fprintf(fp, "\nEOD\n");
    fprintf(fp, "set datafile separator comma\n");
    fprintf(
        fp,
        "plot '$map' matrix rowheaders columnheaders using 1:2:3 with image\n");
    fprintf(fp, "set datafile separator");
    fclose(fp);

    delete[] root_buffer;

    if (call_gnuplot) {
      int res = system("gnuplot affinity.gnuplot > affinity.png");
      printf("Called gnuplot with result %d\n", res);
    }
  }
  delete[] all_masks;

  MPI_Finalize();
  return 0;
}
