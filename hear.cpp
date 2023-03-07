#include <iostream>
#include <mpi.h>

extern "C"

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    std::cout << "MPI_Alleduce() call interception" << std::endl;
    return PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
}

int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm)
{
    std::cout << "MPI_Reduce() call interception" << std::endl;
    return PMPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
}

int MPI_Init(int *argc, char ***argv)
{
    std::cout << "MPI_Init() call interception" << std::endl;
    return PMPI_Init(argc, argv);
}

int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm)
{
    std::cout << "MPI_Comm_create() call interception" << std::endl;
    return PMPI_Comm_create(comm, group, newcomm);
}

int MPI_Finalize()
{
    std::cout << "MPI_Finalize() call interception" << std::endl;
    return PMPI_Finalize();
}
