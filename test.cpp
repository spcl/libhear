#include <mpi.h>

#include <iostream>
#include <vector>
#include <cassert>

const size_t arr_len = 16;
const int magic_num = 42;

int main(int argc, char **argv)
{
    int comm_size;
    std::vector<int> sbuf(arr_len, magic_num);
    std::vector<int> rbuf(arr_len);

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &comm_size);

    MPI_Allreduce(sbuf.data(), rbuf.data(), sbuf.size(),
		  MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    for (const auto &elem: rbuf)
	assert(elem == magic_num * comm_size);

    MPI_Finalize();

    return 0;
}
