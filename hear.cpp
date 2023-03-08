#include <unordered_map>
#include <vector>
#include <random>
#include <iostream>
#include <limits>
#include <mpi.h>

extern "C"

// #define DEBUG

/*
 * WARNING:
 * We rely on (int) + (unsigned int) wrapping up.
 * Need to check standard on casting and integer overflow.
 */
using encr_key_t = unsigned int; /* MPI_UNSIGNED */
const encr_key_t max_encr_key = std::numeric_limits<encr_key_t>::max();

std::random_device rd {};
std::default_random_engine encr_key_generator(rd());
std::uniform_int_distribution<encr_key_t> encr_key_distr(0, std::numeric_limits<encr_key_t>::max());

const int root_rank = 0;

struct HearState
{
public:
    std::vector<std::vector<encr_key_t>> _k_si_storage;
    std::unordered_map<MPI_Comm, std::vector<encr_key_t> *> _k_si_map;
    std::vector<encr_key_t> _k_n_storage;
    std::unordered_map<MPI_Comm, encr_key_t *> _k_n_map;
};

class HearState hear;

static encr_key_t hear_generate_encr_key()
{
    return encr_key_distr(encr_key_generator);
}

static int hear_insert_new_comm(MPI_Comm comm)
{
    int comm_size;
    int my_rank;
    int ret;

    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(comm, &my_rank);

    hear._k_si_storage.push_back(std::vector<encr_key_t>(comm_size, my_rank));
    hear._k_si_storage.back()[my_rank] = hear_generate_encr_key();
    hear._k_si_map.insert({comm, &hear._k_si_storage.back()});
    ret = MPI_Allgather(MPI_IN_PLACE, 1, MPI_UNSIGNED, (*(hear._k_si_map[comm])).data(), 1, MPI_UNSIGNED, comm);
    if (ret != MPI_SUCCESS)
        return ret;

#ifdef DEBUG
    for (auto idx = 0; idx < (*(hear._k_si_map[comm])).size(); idx++) {
        std::cerr << "[DEBUG] rank=" << my_rank << " k_si[" << idx << "]=" << (*(hear._k_si_map[comm]))[idx] << std::endl;
    }
#endif

    hear._k_n_storage.push_back(my_rank == root_rank ? hear_generate_encr_key() : 42);
    hear._k_n_map.insert({comm, &hear._k_n_storage.back()});
    ret = MPI_Bcast(hear._k_n_map[comm], 1, MPI_UNSIGNED, root_rank, comm);
    if (ret != MPI_SUCCESS)
        return ret;

#ifdef DEBUG
    std::cerr << "[DEBUG] rank=" << my_rank << " k_n=" << *(hear._k_n_map[comm]) << std::endl;
#endif

    return MPI_SUCCESS;
}

/* PMPI_* wrappers */

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
#ifdef DEBUG
    std::cerr << "MPI_Alleduce() call interception" << std::endl;
#endif
    return PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);
}

int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm)
{
#ifdef DEBUG
    std::cerr << "MPI_Reduce() call interception" << std::endl;
#endif
    return PMPI_Reduce(sendbuf, recvbuf, count, datatype, op, root, comm);
}

int MPI_Init(int *argc, char ***argv)
{
    int ret;

#ifdef DEBUG
    std::cerr << "MPI_Init() call interception" << std::endl;
#endif
    ret = PMPI_Init(argc, argv);

    /* HEAR init part
     *
     * TODOs:
     * - initialize PRNG
     * - fixed seed in debug mode
     * - parse environment variables if there are any
     */

    if (ret == MPI_SUCCESS)
        ret = hear_insert_new_comm(MPI_COMM_WORLD);

    return ret;
}

int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *newcomm)
{
    int ret;

#ifdef DEBUG
    std::cerr << "MPI_Comm_create() call interception" << std::endl;
#endif
    ret = PMPI_Comm_create(comm, group, newcomm);

    if (ret == MPI_SUCCESS)
        ret = hear_insert_new_comm(*newcomm);

    return ret;
}

int MPI_Finalize()
{
#ifdef DEBUG
    std::cerr << "MPI_Finalize() call interception" << std::endl;
#endif
    return PMPI_Finalize();
}
