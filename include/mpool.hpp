#ifndef MPOOL_HPP
#define MPOOL_HPP

#include <cstddef>
#include <list>

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

#endif
