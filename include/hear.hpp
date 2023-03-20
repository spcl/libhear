#ifndef HEAR_HPP
#define HEAR_HPP

#include <mpi.h>

extern "C"

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
int MPI_Init(int *argc, char ***argv);
int MPI_Init_thread(int *argc, char ***argv, int required, int *provided);
int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm);
int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm);
int MPI_Finalize();

#endif
