#ifndef ENCRYPT_HPP
#define ENCRYPT_HPP

#include <vector>
#include <random>

using encr_key_t = unsigned int;

extern const encr_key_t max_encr_key;
extern std::random_device rd;
extern std::mt19937 encr_key_generator;
extern std::mt19937 encr_noise_generator;
extern std::uniform_int_distribution<encr_key_t> encr_key_distr;

encr_key_t generate_encr_key();
encr_key_t prng(encr_key_t seed);

template<typename T, typename K>
inline void __encrypt(T encr_sbuf, K sbuf, int count, int rank,
                      std::vector<encr_key_t> &k_s, encr_key_t k_n)
{
    encr_key_t key = k_n + k_s[rank];
    for (auto i = 0; i < count; i++) {
        encr_sbuf[i] = sbuf[i] + prng(key + i);
    }
}

template<typename T>
inline void __decrypt(T rbuf, int count, std::vector<encr_key_t> &k_s, encr_key_t k_n)
{
    encr_key_t key;

    for (auto i = 0; i < count; i++) {
        key = k_n + i;
        for (auto j = 0; j < k_s.size(); j++) {
            rbuf[i] = rbuf[i] - prng(key + k_s[j]);
        }
    }
}

#endif
