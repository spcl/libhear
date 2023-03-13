#include <unordered_map>
#include <vector>
#include <random>
#include <iostream>
#include <limits>
#include <cassert>
#include <cstring>
#include <list>

#include <mpi.h>

extern "C"

#define USE_MPOOL
#define USE_PIPELINING
// #define DCHECK

#ifdef USE_PIPELINING
const int pipelining_block_size = 65536;
#endif

/*
 * We need at least two pre-allocated buffers to enable pipelining,
 * i.e., memcopy + de-/en-cryption overlapped with communication.
 */

#ifdef USE_MPOOL
const size_t mpool_size = 64;
const size_t mpool_sbuf_len = 1048576;
#endif

using encr_key_t = unsigned int; /* MPI_UNSIGNED */
const encr_key_t max_encr_key = 42; // std::numeric_limits<encr_key_t>::max();

std::random_device rd {};
std::mt19937 encr_key_generator(rd());
std::mt19937 encr_noise_generator;
std::uniform_int_distribution<encr_key_t> encr_key_distr(0, max_encr_key);

const int root_rank = 0;

static inline encr_key_t __generate_encr_key()
{
    return encr_key_distr(encr_key_generator);
}

static inline encr_key_t __prng(encr_key_t seed)
{
    encr_noise_generator.seed(seed);
    return encr_key_distr(encr_noise_generator);
    /*
     * Warning:
     * signed integer overflow is UB.
     * TODO: workaround with casting int to uint and do all the stuff safel.
     *
     //    return encr_noise_generator();
     *
     */
}

template<typename T, typename K>
static inline void __encrypt(T encr_sbuf, K sbuf, int count, int rank,
                      std::vector<encr_key_t> &k_s, encr_key_t k_n)
{
    for (auto i = 0; i < count; i++) {
        encr_sbuf[i] = sbuf[i] + __prng(k_n + k_s[rank] + i);
    }
}

template<typename T>
static inline void __decrypt(T rbuf, int count, std::vector<encr_key_t> &k_s, encr_key_t k_n)
{
    int my_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    for (auto i = 0; i < count; i++) {
        for (auto j = 0; j < k_s.size(); j++) {
            rbuf[i] = rbuf[i] - __prng(k_n + k_s[j] + i);
        }
    }
}

struct HearMpool
{

private:

    size_t _buf_len;
    std::list<char *> _mpool;

    void cleanup();

public:

    HearMpool() = default;
    HearMpool(const size_t pool_size, const size_t buf_len);
    ~HearMpool();

    void* acquire_buf();
    void release_buf(void *buf);

};

HearMpool::HearMpool(const size_t pool_size, const size_t buf_len)
    : _buf_len(buf_len)
{
    char *ptr;

    for (auto i = 0; i < pool_size; i++) {
        ptr = new char[buf_len];
        if (ptr == nullptr) {
            cleanup();
            std::cerr << "Failed to allocate memory for mpool" << std::endl;
            exit(EXIT_FAILURE);
        }

        _mpool.push_back(ptr);
    }
}

HearMpool::~HearMpool()
{
    cleanup();
}

void HearMpool::cleanup()
{
    char *ptr;

    while ((ptr = reinterpret_cast<char *>(acquire_buf())) != nullptr) {
        delete[] ptr;
    }
}

inline void* HearMpool::acquire_buf()
{
    void *t = nullptr;

    if (!_mpool.empty()) {
         t = _mpool.back();
         _mpool.pop_back();
    }

    return t;
}

inline void HearMpool::release_buf(void *buf)
{
    assert(buf);
    _mpool.push_back(reinterpret_cast<char *>(buf));
}

struct HearState
{

private:

    std::vector<std::vector<encr_key_t>> _k_s_storage;
    std::unordered_map<MPI_Comm, std::vector<encr_key_t> *> _k_s_map;
    std::vector<encr_key_t> _k_n_storage;
    std::unordered_map<MPI_Comm, encr_key_t *> _k_n_map;
    HearMpool _sbuf_mpool;

public:

    HearState();
    ~HearState();

    void release_memory(void *buf);
    int insert_new_comm(MPI_Comm comm);
    void update_k_n(MPI_Comm comm);
    void* encrypt_sendbuf(const void *sendbuf, void *recvbuf, int count,
                          MPI_Datatype datatype, MPI_Op op, MPI_Comm comm);
    int decrypt_recvbuf(void *recvbuf, int count, MPI_Datatype datatype,
                        MPI_Op op, MPI_Comm comm);

};

class HearState *hear;

HearState::HearState()
#ifdef USE_MPOOL
    : _sbuf_mpool(mpool_size, mpool_sbuf_len)
#endif
{

}

HearState::~HearState()
{

}

inline void HearState::update_k_n(MPI_Comm comm)
{
    *(_k_n_map[comm]) = __prng(*(_k_n_map[comm]));
}

inline int HearState::insert_new_comm(MPI_Comm comm)
{
    int comm_size;
    int my_rank;
    int ret;

    MPI_Comm_size(comm, &comm_size);
    MPI_Comm_rank(comm, &my_rank);

    hear->_k_s_storage.push_back(std::vector<encr_key_t>(comm_size, my_rank));
    hear->_k_s_storage.back()[my_rank] = __generate_encr_key();
    hear->_k_s_map.insert({comm, &hear->_k_s_storage.back()});
    ret = MPI_Allgather(MPI_IN_PLACE, 1, MPI_UNSIGNED, (*(hear->_k_s_map[comm])).data(), 1, MPI_UNSIGNED, comm);
    if (ret != MPI_SUCCESS)
        return ret;

    hear->_k_n_storage.push_back(my_rank == root_rank ? __generate_encr_key() : 42);
    hear->_k_n_map.insert({comm, &hear->_k_n_storage.back()});
    ret = MPI_Bcast(hear->_k_n_map[comm], 1, MPI_UNSIGNED, root_rank, comm);
    if (ret != MPI_SUCCESS)
        return ret;

    return MPI_SUCCESS;
}

inline void* HearState::encrypt_sendbuf(const void *sendbuf, void *recvbuf, int count,
                                        MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
    void *encr_sbuf = nullptr;
    int type_size;
    int sbuf_len;
    int my_rank;

    MPI_Comm_rank(comm, &my_rank);
    MPI_Type_size(datatype, &type_size);
    sbuf_len = count * type_size;

#ifndef USE_MPOOL
    encr_sbuf = new char[sbuf_len];
#else
    assert(sbuf_len <= mpool_sbuf_len);
    encr_sbuf = _sbuf_mpool.acquire_buf();
#endif
    if (encr_sbuf == nullptr) {
        return nullptr;
    }

    /* 3ncrypt10n */
    if (datatype == MPI_INT) {
        __encrypt<int *, const int *>(reinterpret_cast<int *>(encr_sbuf),
                                      reinterpret_cast<const int *>(sendbuf), count, my_rank,
                                      *hear->_k_s_map[comm], *hear->_k_n_map[comm]);
    } else {
        std::cerr << "Encryption for this MPI datatype is not supported!" << std::endl;
#ifndef USE_MPOOL
        delete[] reinterpret_cast<char *>(encr_sbuf);
#else
        _sbuf_mpool.release_buf(encr_sbuf);
#endif
        return nullptr;
    }

    return encr_sbuf;
}

inline int HearState::decrypt_recvbuf(void *recvbuf, int count, MPI_Datatype datatype,
                                      MPI_Op op, MPI_Comm comm)
{
    /* d3crypt10n */
    if (datatype == MPI_INT) {
        __decrypt<int *>(reinterpret_cast<int *>(recvbuf), count,
                         *hear->_k_s_map[comm], *hear->_k_n_map[comm]);
    } else {
        std::cerr << "Encryption for this MPI datatype is not supported!" << std::endl;
        return MPI_ERR_TYPE;
    }

    return MPI_SUCCESS;
}

inline void HearState::release_memory(void *buf)
{
    assert(buf);
#ifndef USE_MPOOL
    delete[] static_cast<char *>(buf);
#else
    _sbuf_mpool.release_buf(buf);
#endif
}

/*
 * PMPI_* wrappers
 */

int MPI_Allreduce(const void *sendbuf, void *recvbuf, int count,
                  MPI_Datatype datatype, MPI_Op op, MPI_Comm comm)
{
#ifdef USE_PIPELINING
    MPI_Request req;
    MPI_Status status;
    int prev_offset, cur_offset, next_offset;
    int prev_count, cur_count, next_count, total_count;
    void *encr_sendbuf_next;
#endif
    void *encr_sendbuf;
    int dtype_size;
    int ret;

    if ((datatype != MPI_INT) && (op != MPI_SUM))
        return PMPI_Allreduce(sendbuf, recvbuf, count, datatype, op, comm);

    MPI_Type_size(datatype, &dtype_size);

#ifdef DCHECK
    void *valid_rbuf = new char[dtype_size * count];
    assert(valid_rbuf);
    PMPI_Allreduce(sendbuf, valid_rbuf, count, datatype, op, comm);
#endif

#ifdef DEBUG
    std::cerr << "MPI_Alleduce() call interception" << std::endl;
#endif

    /*
     * Encryption logic starts here
     */

    hear->update_k_n(comm);

#ifndef USE_PIPELINING
    encr_sendbuf = hear->encrypt_sendbuf(sendbuf, recvbuf, count, datatype, op, comm);
    if (encr_sendbuf == nullptr)
        return MPI_ERR_BUFFER;

    ret = PMPI_Allreduce(encr_sendbuf, recvbuf, count, datatype, op, comm);
    if (ret != MPI_SUCCESS)
        goto cleanup;

    ret = hear->decrypt_recvbuf(recvbuf, count, datatype, op, comm);
    if (ret != MPI_SUCCESS)
        goto cleanup;

    hear->release_memory(encr_sendbuf);
#else
    /*
     * Overlap communication of n'th block with decryption of
     * n-1'th block and encryption of n+1'th block
     */
    total_count = count;
    cur_count = total_count < pipelining_block_size ? total_count : pipelining_block_size;
    prev_offset = cur_offset = next_offset = 0;

    encr_sendbuf = hear->encrypt_sendbuf(sendbuf, recvbuf, cur_count, datatype, op, comm);
    if (!encr_sendbuf) {
        ret = MPI_ERR_BUFFER;
        goto cleanup;
    }

    while (total_count) {
        ret = MPI_Iallreduce(encr_sendbuf, reinterpret_cast<char *>(recvbuf) + cur_offset, cur_count,
                             datatype, op, comm, &req);
        if (ret != MPI_SUCCESS)
            goto cleanup;

        if (cur_offset > 0) {
            ret = hear->decrypt_recvbuf(reinterpret_cast<char *>(recvbuf) + prev_offset, prev_count,
                                        datatype, op, comm);
            if (ret != MPI_SUCCESS) {
                goto cleanup;
            }
        }

        total_count -= cur_count;

        if (total_count) {
            next_offset += cur_count * dtype_size;
            next_count = total_count < pipelining_block_size ? total_count : pipelining_block_size;
            encr_sendbuf_next = hear->encrypt_sendbuf(reinterpret_cast<const char *>(sendbuf) + next_offset,
                                                      reinterpret_cast<char *>(recvbuf) + next_offset,
                                                      next_count, datatype, op, comm);
            if (!encr_sendbuf_next) {
                ret = MPI_ERR_BUFFER;
                goto cleanup;
            }
        }

        MPI_Wait(&req, MPI_STATUS_IGNORE);

        prev_count = cur_count;
        prev_offset = cur_offset;

        cur_count = next_count;
        cur_offset = next_offset;

        hear->release_memory(encr_sendbuf);
        encr_sendbuf = encr_sendbuf_next;
    }

    ret = hear->decrypt_recvbuf(reinterpret_cast<char *>(recvbuf) + prev_offset, prev_count, datatype, op, comm);
    if (ret != MPI_SUCCESS)
        return ret;
#endif

#ifdef DCHECK
    assert(!std::memcmp(valid_rbuf, recvbuf, dtype_size * count));
#endif

    return MPI_SUCCESS;

cleanup:
#ifdef DCHECK
    delete[] reinterpret_cast<char *>(valid_rbuf);
#endif
    hear->release_memory(encr_sendbuf);
    return ret;
}

int MPI_Init(int *argc, char ***argv)
{
    int ret;

#ifdef DEBUG
    std::cerr << "MPI_Init() call interception" << std::endl;
#endif
    ret = PMPI_Init(argc, argv);

    hear = new HearState();
    assert(hear);

    if (ret == MPI_SUCCESS)
        ret = hear->insert_new_comm(MPI_COMM_WORLD);

    return ret;
}

int MPI_Init_thread(int *argc, char ***argv, int required, int *provided)
{
    int ret;

#ifdef DEBUG
    std::cerr << "MPI_Init_thread() call interception" << std::endl;
#endif
    ret = PMPI_Init_thread(argc, argv, required, provided);

    /*
     * TODO: add HearState protection
     */
    hear = new HearState();
    assert(hear);

    if (ret == MPI_SUCCESS)
        ret = hear->insert_new_comm(MPI_COMM_WORLD);

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
        ret = hear->insert_new_comm(*newcomm);

    return ret;
}

int MPI_Comm_split(MPI_Comm comm, int color, int key, MPI_Comm *newcomm)
{
    int ret;

#ifdef DEBUG
    std::cerr << "MPI_Comm_split() call interception" << std::endl;
#endif
    ret = PMPI_Comm_split(comm, color, key, newcomm);
    if (ret == MPI_SUCCESS)
        ret = hear->insert_new_comm(*newcomm);

    return ret;
}

int MPI_Finalize()
{
#ifdef DEBUG
    std::cerr << "MPI_Finalize() call interception" << std::endl;
#endif
    delete hear;

    return PMPI_Finalize();
}
