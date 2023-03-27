#ifndef ENCRYPT_HPP
#define ENCRYPT_HPP

#include <vector>
#include <random>

#ifdef AESNI
#include <immintrin.h>
#include <wmmintrin.h>
#endif

namespace encryption {

using encr_key_t = unsigned int;

extern std::mt19937 encr_noise_generator;

inline unsigned int prng(unsigned int seed)
{
    encr_noise_generator.seed(seed);
    return encr_noise_generator();
}

void encrypt_int_sum_naive(unsigned int *encr_sbuf, const unsigned int *sbuf, int count, int rank,
			   std::vector<unsigned int> &k_s, unsigned int k_n, bool is_edge);
void decrypt_int_sum_naive(unsigned int *rbuf, int count,
			   std::vector<unsigned int> &k_s, unsigned int k_n);
void encrypt_int_prod_naive(unsigned int *encr_sbuf, const unsigned int *sbuf, int count, int rank,
			   std::vector<unsigned int> &k_s, unsigned int k_n, bool is_edge);
void decrypt_int_prod_naive(unsigned int *rbuf, int count,
			    std::vector<unsigned int> &k_s, unsigned int k_n);
void encrypt_float_sum_naive(float *encr_sbuf, const float *sbuf, int count, int rank,
			     std::vector<unsigned int> &k_s, unsigned int k_n);
void decrypt_float_sum_naive(float *rbuf, int count, std::vector<unsigned int> &k_s, unsigned int k_n);

#ifdef AESNI

unsigned int aesni128_prng(unsigned int);
void aesni128_load_key(char *enc_key);

void encrypt_int_sum_aesni128(unsigned int *encr_sbuf, const unsigned int *sbuf, int count, int rank,
			      std::vector<unsigned int> &k_s, unsigned int k_n, bool is_edge);
void decrypt_int_sum_aesni128(unsigned int *rbuf, int count, std::vector<unsigned int> &k_s, unsigned int k_n);

#endif

}

#endif
