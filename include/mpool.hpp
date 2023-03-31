#ifndef MPOOL_HPP
#define MPOOL_HPP

#include <cstddef>
#include <list>

namespace mpool {

struct SbufMpool
{

private:

    size_t _buf_len;
    std::list<void *> _mpool;

    void cleanup();

public:

    SbufMpool() = default;
    SbufMpool(const size_t pool_size, const size_t buf_len);
    ~SbufMpool();

    void* acquire_buf();
    void release_buf(void *buf);

};

}

#endif
