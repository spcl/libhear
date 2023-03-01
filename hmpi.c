#include <stdio.h>
#include <mpi.h>

#define HMPI_DEBUG 1
#define HMPI_MPOOL_MAX_SIZE 32

struct mpool_list
{
    char *buf;
    struct mpool_list *next;
};

struct hmpi_state
{
    /* TODO: add thread safety, i.e. wrap up accesses with mutex */
    /* memory pool list accumulates allocated memory for intermediate encryption buffer */
    struct mpool_list *mpool;
    int mpool_len;

    char *lkey;
    char *rkey;
};

static hmpi_state hmpi_ctx;

int alloc_sbuf(int count, MPI_Datatype datatype, void **sbuf)
{
    int dtype_size;
    char *buf;
   
    MPI_Type_size(datatype, &dtype_size);
    
    if (hmpi_ctx.mpool == NULL) {
        assert(hmpi_ctx.mpool_len == 0);
        buf = malloc(dtype_size * count);
    } else {
        /* TODO: dispatch buffer from mpool */
        hmpi_ctx.mpool_len--;
    }

    if (buf == NULL)
        return MPI_ERR_BUFFER;
    

    *sbuf = (void *)buf;

    return MPI_SUCCESS;
}

void free_sendbuf(void *sbuf)
{
    assert(sbuf);

    if (mpool_len < HMPI_MPOOL_MAX_SIZE) {
        mpool_len++;
        /* TODO: add sbuf to mempool */
    }

    free(sbuf);
}

int encrypt_data(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype, int root)
{
    /* TODO: encryption logic */
    /* TODO: handle MPI_IN_PLACE */
    
    return MPI_SUCCESS;
}

int decrypt_data(void *recvbuf, int count, MPI_Datatype datatype)
{
    /* TODO: decryption logic */
    MPI_SUCCESS;
}

/* MPI 3.0 added const to input i.e. read-only arguments... */
#if MPI_VERSION >= 3
int MPI_Reduce(const void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm)
#else
int MPI_Reduce(void *sendbuf, void *recvbuf, int count, MPI_Datatype datatype,
               MPI_Op op, int root, MPI_Comm comm)
#endif
{
    void *encrypted_sbuf;
    int ret;

    ret = alloc_sbuf(count, datatype, &encrypted_sbuf);
    if (ret != MPI_SUCCESS)
        return ret;

    ret = encrypt_data(sendbuf, recvbuf, count, datatype, root, encrypted_sbuf);
    if (ret != MPI_SUCCESS)
        goto free_data;

#ifdef HMPI_DEBUG
    fprintf(stderr, "Calling HMPI_Reduce");
#endif
    ret = PMPI_Reduce(encrypted_sbuf, recvbuf, count, datatype, op, root, comm);
    if (ret != MPI_SUCCESS)
        goto free_data;

    ret = decrypt_data(recvbuf, count, datatype);

free_data:
    free_sendbuf(encrypted_sbuf);
    
    return ret;
}

int MPI_Init(int *argc, char ***argv)
{
#ifdef HMPI_DEBUG
    fprintf(stderr, "Calling HMPI_Init");
#endif
    PMPI_Init(argc, argv);

    /* initialize hmpi_ctx here */
    hmpi_ctx.mpool = NULL;
    hmpi_ctx.mpool_len = 0;

    /* TODO: allocate memory for l-/r-keys */
    /* TODO: l-/r-keys exchange */
}

int MPI_Finalize()
{
#ifdef HMPI_DEBUG
    fprintf(stderr, "Calling HMPI_Finalize");
#endif
    PMPI_Finalize(argc, argv);

    /* TODO: cleanup hmpi_ctx here, i.e. mempool, l-/r-keys */
}
