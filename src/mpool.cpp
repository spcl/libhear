#include <iostream>
#include <cassert>

#include "mpool.hpp"

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

void* HearMpool::acquire_buf()
{
    void *t = nullptr;

    if (!_mpool.empty()) {
         t = _mpool.back();
         _mpool.pop_back();
    }

    return t;
}

void HearMpool::release_buf(void *buf)
{
    assert(buf);
    _mpool.push_back(reinterpret_cast<char *>(buf));
}
