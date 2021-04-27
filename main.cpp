#include <mpi.h>
#include <stdio.h>
#include <assert.h>
#include <sched.h>
#include <sys/sysinfo.h>
#include <string>

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);

    int size, rank;
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0)
    {
        printf("MPI Size: %d\n", size);
    }
    MPI_Barrier(MPI_COMM_WORLD);

    // get process affinity
    cpu_set_t mask;
    long nproc, i;

    CPU_ZERO(&mask);
    assert(sched_getaffinity(0, sizeof(cpu_set_t), &mask) == 0);
    nproc = get_nprocs();

    // generate description based on affinity
    char buffer[1024] = {0};
    int last_zero = -1;
    for (i = 0; i <= nproc; i++)
    {
        if (i == nproc || !CPU_ISSET(i, &mask))
        {
            if (i > last_zero + 1)
            {
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

    char *root_buffer = NULL;
    if (rank == 0)
    {
        root_buffer = new char[sizeof(buffer) * size];
    }
    MPI_Gather(buffer, sizeof(buffer), MPI_CHAR, root_buffer, sizeof(buffer), MPI_CHAR, 0, MPI_COMM_WORLD);
    if (rank == 0)
    {
        for (int i = 0; i < size; i++)
        {
            printf("Rank %d:%s\n", i, &root_buffer[i * sizeof(buffer)]);
        }
        delete [] root_buffer;
    }

    MPI_Finalize();
    return 0;
}