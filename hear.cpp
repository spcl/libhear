#include <unordered_map>
#include <vector>
#include <random>
#include <iostream>
#include <limits>
#include <cassert>
#include <cstring>

#include <mpi.h>

extern "C"

//#define DEBUG

/*
 * WARNING:
 * We rely on (int) + (unsigned int) wrapping up.
 * Need to check standard on casting and integer overflow.
 * UPD: signed integer overflow is UB.
 * TODO: workaround with casting int to uint and do all the stuff safely.
 */
using encr_key_t = unsigned int; /* MPI_UNSIGNED */
const encr_key_t max_encr_key = 42; //std::numeric_limits<encr_key_t>::max();

std::random_device rd {};
std::mt19937 encr_key_generator(rd());
std::mt19937 encr_noise_generator;
std::uniform_int_distribution<encr_key_t> encr_key_distr(0, max_encr_key);

const int root_rank = 0;

struct HearState
{
public:
    std::vector<std::vector<encr_key_t>> _k_s_storage;
    std::unordered_map<MPI_Comm, std::vector<encr_key_t> *> _k_s_map;
    std::vector<encr_key_t> _k_n_storage;
    std::unordered_map<MPI_Comm, encr_key_t *> _k_n_map;
};

class HearState hear;

static encr_key_t __generate_encr_key()
{
    return encr_key_distr(encr_key_generator);
}

static encr_key_t __prng(encr_key_t seed)
{
    encr_noise_generator.seed(seed);
    return encr_key_distr(encr_noise_generator);
}

template<typename T, typename K>
static void __encrypt(T encr_sbuf, K sbuf, int count, int rank,
                      std::vector<encr_key_t> &k_s, encr_key_t k_n)
{
    for (auto i = 0; i < count; i++) {
        encr_sbuf[i] = sbuf[i] + __prng(k_n + k_s[rank] + i);
    }
}

template<typename T>
static void __decrypt(T rbuf, int count, std::vector<encr_key_t> &k_s, encr_key_t k_n)
{
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    for (auto i = 0; i < count; i++) {
        for (auto j = 0; j < k_s.size(); j++) {
            rbuf[i] = rbuf[i] - __prng(k_n + k_s[j] + i);
        }
    }
}

static int hear_insert_new_comm(MPI_Comm comm)
{
    int comm_size;
    int my_rank;
    int ret;

    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(comm, &my_rank);

    hear._k_s_storage.push_back(std::vector<encr_key_t>(comm_size, my_rank));
    hear._k_s_storage.back()[my_rank] = __generate_encr_key();
    hear._k_s_map.insert({comm, &hear._k_s_storage.back()});
    ret = MPI_Allgather(MPI_IN_PLACE, 1, MPI_UNSIGNED, (*(hear._k_s_map[comm])).data(), 1, MPI_UNSIGNED, comm);
    if (ret != MPI_SUCCESS)
        return ret;

#ifdef DEBUG
    for (auto idx = 0; idx < (*(hear._k_s_map[comm])).size(); idx++) {
        std::cerr << "[DEBUG] rank=" << my_rank << " k_s[" << idx << "]=" << (*(hear._k_s_map[comm]))[idx] << std::endl;
    }
#endif

    hear._k_n_storage.push_back(my_rank == root_rank ? __generate_encr_key() : 42);
    hear._k_n_map.insert({comm, &hear._k_n_storage.back()});
    ret = MPI_Bcast(hear._k_n_map[comm], 1, MPI_UNSIGNED, root_rank, comm);
    if (ret != MPI_SUCCESS)
        return ret;

#ifdef DEBUG
    std::cerr << "[DEBUG] rank=" << my_rank << " k_n=" << *(hear._k_n_map[comm]) << std::endl;
#endif

    return MPI_SUCCESS;
}

static void* hear_encrypt_sendbuf(const void *sendbuf, void *recvbuf, int count,
                                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    char *encr_sbuf = nullptr;
    int type_size;
    int sbuf_len;
    int my_rank;

    MPI_Type_size(datatype, &type_size);
    sbuf_len = count * type_size;

    encr_sbuf = new char[sbuf_len];
    if (encr_sbuf == nullptr)
        return nullptr;

    /* 3ncrypt10n */
    if (datatype == MPI_INT) {
        *hear._k_n_map[comm] = __prng(*hear._k_n_map[comm]);
        MPI_Comm_rank(comm, &my_rank);
        __encrypt<int *, const int *>(reinterpret_cast<int *>(encr_sbuf),
                                      reinterpret_cast<const int *>(sendbuf), count, my_rank,
                                      *hear._k_s_map[comm], *hear._k_n_map[comm]);
    } else {
        std::cerr << "Encryption for this MPI datatype is not supported!" << std::endl;
        delete[] encr_sbuf;
        return nullptr;
    }

    return static_cast<void *>(encr_sbuf);
}

static int hear_decrypt_recvbuf(void *recvbuf, int count, MPI_Datatype datatype,
                                MPI_Op op, MPI_Comm comm)
{
    /* d3crypt10n */
    if (datatype == MPI_INT) {
        __decrypt<int *>(reinterpret_cast<int *>(recvbuf), count,
                         *hear._k_s_map[comm], *hear._k_n_map[comm]);
    } else {
        std::cerr << "Encryption for this MPI datatype is not supported!" << std::endl;
        return MPI_ERR_TYPE;
    }

    return MPI_SUCCESS;
}

static void hear_free_memory(void *buf)
{
    assert(buf);

    delete[] static_cast<char *>(buf);
}

/* PMPI_* wrappers */

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    void *encr_sendbuf;
    int ret;

    encr_sendbuf = hear_encrypt_sendbuf(sendbuf, recvbuf, count, datatype, op, comm);
    if (encr_sendbuf == nullptr)
        return MPI_ERR_BUFFER;

#ifdef DEBUG
    std::cerr << "MPI_Alleduce() call interception" << std::endl;
#endif
    ret = PMPI_Allreduce(encr_sendbuf, recvbuf, count, datatype, op, comm);
    if (ret == MPI_SUCCESS)
        ret = hear_decrypt_recvbuf(recvbuf, count, datatype, op, comm);

cleanup:
    hear_free_memory(encr_sendbuf);

    return ret;
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
