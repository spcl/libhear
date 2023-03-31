#include <cassert>

#include "encrypt.hpp"

namespace encryption {

std::mt19937 encr_noise_generator;

void encrypt_int_sum_naive(unsigned int *encr_sbuf, const unsigned int *sbuf, int count, int rank,
			   std::vector<unsigned int> &k_s, unsigned int k_n, bool is_edge)
{
    unsigned int tmp1 = k_n + k_s[rank];
    unsigned int tmp2 = k_n + k_s[rank + 1];

    if (!is_edge) {
	for (unsigned int i = 0; i < count; i++) {
	    encr_sbuf[i] = sbuf[i] + prng_uint(tmp1 + i) - prng_uint(tmp2 + i);
	}
    } else {
	for (unsigned int i = 0; i < count; i++) {
	    encr_sbuf[i] = sbuf[i] + prng_uint(tmp1 + i);
	}
    }
}

void decrypt_int_sum_naive(unsigned int *rbuf, int count,
			   std::vector<unsigned int> &k_s, unsigned int k_n)
{
    unsigned int tmp = k_n + k_s[0];

    for (unsigned int i = 0; i < count; i++) {
	rbuf[i] = rbuf[i] - prng_uint(tmp + i);
    }
}

void encrypt_int_sum_sha1sse2(unsigned int *encr_sbuf, const unsigned int *sbuf, int count, int rank,
			      std::vector<unsigned int> &k_s, unsigned int k_n, bool is_edge)
{
    unsigned int tmp1 = k_n + k_s[rank];
    unsigned int tmp2 = k_n + k_s[rank + 1];
    __m128i ind1 = _mm_set_epi32(3 + tmp1, 2 + tmp1, 1 + tmp1, tmp1);
    __m128i ind2 = _mm_set_epi32(3 + tmp2, 2 + tmp2, 1 + tmp2, tmp2);
    __m128i incr = _mm_set_epi32(4, 4, 4, 4);
    __m128i encr_sbuf_vec, noise1, noise2;

    if (!is_edge) {
	for (unsigned int i = 0; i < count; i+=4) {
	    encr_sbuf_vec = _mm_load_si128(reinterpret_cast<const __m128i*>(sbuf + i));
	    noise1 = prng_m128(ind1);
	    noise2 = prng_m128(ind2);
	    encr_sbuf_vec = _mm_add_epi32(encr_sbuf_vec, noise1);
	    encr_sbuf_vec = _mm_sub_epi32(encr_sbuf_vec, noise2);
	    _mm_store_si128(reinterpret_cast<__m128i*>(encr_sbuf + i), encr_sbuf_vec);
	    ind1 = _mm_add_epi32(ind1, incr);
	    ind2 = _mm_add_epi32(ind2, incr);
	}
    } else {
	for (unsigned int i = 0; i < count; i+=4) {
	    encr_sbuf_vec = _mm_load_si128(reinterpret_cast<const __m128i*>(sbuf + i));
	    noise1 = prng_m128(ind1);
	    encr_sbuf_vec = _mm_add_epi32(encr_sbuf_vec, noise1);
	    _mm_store_si128(reinterpret_cast<__m128i*>(encr_sbuf + i), encr_sbuf_vec);
	    ind1 = _mm_add_epi32(ind1, incr);
	}
    }
}

void decrypt_int_sum_sha1sse2(unsigned int *rbuf, int count, std::vector<unsigned int> &k_s, unsigned int k_n)
{
    unsigned int tmp = k_n + k_s[0];
    __m128i ind  = _mm_set_epi32(3 + tmp, 2 + tmp, 1 + tmp, tmp);
    __m128i incr = _mm_set_epi32(4, 4, 4, 4);
    __m128i decr_rbuf_vec, noise;

    for (unsigned int i = 0; i < count; i+=4) {
	decr_rbuf_vec = _mm_load_si128(reinterpret_cast<const __m128i*>(rbuf + i));
	noise = prng_m128(ind);
	decr_rbuf_vec = _mm_sub_epi32(decr_rbuf_vec, noise);
	_mm_store_si128(reinterpret_cast<__m128i*>(rbuf + i), decr_rbuf_vec);
	ind = _mm_add_epi32(ind, incr);
    }
}

void encrypt_int_sum_sha1avx2(unsigned int *encr_sbuf, const unsigned int *sbuf, int count, int rank,
			      std::vector<unsigned int> &k_s, unsigned int k_n, bool is_edge)
{
    unsigned int tmp1 = k_n + k_s[rank];
    unsigned int tmp2 = k_n + k_s[rank + 1];
    __m256i ind1 = _mm256_set_epi32(7 + tmp1, 6 + tmp1, 5 + tmp1, 4 + tmp1, 3 + tmp1, 2 + tmp1, 1 + tmp1, tmp1);
    __m256i ind2 = _mm256_set_epi32(7 + tmp1, 6 + tmp1, 5 + tmp1, 4 + tmp1, 3 + tmp1, 2 + tmp1, 1 + tmp1, tmp1);
    __m256i incr = _mm256_set_epi32(8, 8, 8, 8, 8, 8, 8, 8);
    __m256i encr_sbuf_vec, noise1, noise2;

    if (!is_edge) {
	for (unsigned int i = 0; i < count; i+=8) {
	    encr_sbuf_vec = _mm256_load_si256(reinterpret_cast<const __m256i*>(sbuf + i));
	    noise1 = prng_m256(ind1);
	    noise2 = prng_m256(ind2);
	    encr_sbuf_vec = _mm256_add_epi32(encr_sbuf_vec, noise1);
	    encr_sbuf_vec = _mm256_sub_epi32(encr_sbuf_vec, noise2);
	    _mm256_store_si256(reinterpret_cast<__m256i*>(encr_sbuf + i), encr_sbuf_vec);
	    ind1 = _mm256_add_epi32(ind1, incr);
	    ind2 = _mm256_add_epi32(ind2, incr);
	}
    } else {
	for (unsigned int i = 0; i < count; i+=8) {
	    encr_sbuf_vec = _mm256_load_si256(reinterpret_cast<const __m256i*>(sbuf + i));
	    noise1 = prng_m256(ind1);
	    encr_sbuf_vec = _mm256_add_epi32(encr_sbuf_vec, noise1);
	    _mm256_store_si256(reinterpret_cast<__m256i*>(encr_sbuf + i), encr_sbuf_vec);
	    ind1 = _mm256_add_epi32(ind1, incr);
	}
    }
}

void decrypt_int_sum_sha1avx2(unsigned int *rbuf, int count, std::vector<unsigned int> &k_s, unsigned int k_n)
{
    unsigned int tmp = k_n + k_s[0];
    __m256i ind  = _mm256_set_epi32(7 + tmp, 6 + tmp, 5 + tmp, 4 + tmp, 3 + tmp, 2 + tmp, 1 + tmp, tmp);
    __m256i incr = _mm256_set_epi32(8, 8, 8, 8, 8, 8, 8, 8);
    __m256i decr_rbuf_vec, noise;

    for (unsigned int i = 0; i < count; i+=8) {
	decr_rbuf_vec = _mm256_load_si256(reinterpret_cast<const __m256i*>(rbuf + i));
	noise = prng_m256(ind);
	decr_rbuf_vec = _mm256_sub_epi32(decr_rbuf_vec, noise);
	_mm256_store_si256(reinterpret_cast<__m256i*>(rbuf + i), decr_rbuf_vec);
	ind = _mm256_add_epi32(ind, incr);
    }
}

void encrypt_int_prod_naive(unsigned int *encr_sbuf, const unsigned int *sbuf, int count, int rank,
			    std::vector<unsigned int> &k_s, unsigned int k_n, bool is_edge)
{
    /* Implement me */
}

void decrypt_int_prod_naive(unsigned int *rbuf, int count,
			    std::vector<unsigned int> &k_s, unsigned int k_n)
{
    /* Implement me */
}

void encrypt_float_sum_naive(float *encr_sbuf, const float *sbuf, int count, int rank,
			     std::vector<unsigned int> &k_s, unsigned int k_n)
{
    /* Implement me */
}

void decrypt_float_sum_naive(float *rbuf, int count, std::vector<unsigned int> &k_s, unsigned int k_n)
{
    /* Implement me */
}

#ifdef AESNI

static __m128i key_schedule[11];

#define AESNI128_KEY_EXPAND(k, rcon) \
    (aesni128_key_expand(k, _mm_aeskeygenassist_si128(k, rcon)))

#define AESNI128_ENC_BLOCK(m, n, k)	    \
    do {				    \
        n = _mm_xor_si128(m, k[0]);	    \
        n = _mm_aesenc_si128(m, k[1]);	    \
        n = _mm_aesenc_si128(m, k[2]);	    \
        n = _mm_aesenc_si128(m, k[3]);	    \
        n = _mm_aesenc_si128(m, k[4]);	    \
        n = _mm_aesenc_si128(m, k[5]);	    \
        n = _mm_aesenc_si128(m, k[6]);	    \
        n = _mm_aesenc_si128(m, k[7]);	    \
        n = _mm_aesenc_si128(m, k[8]);	    \
        n = _mm_aesenc_si128(m, k[9]);	    \
        n = _mm_aesenclast_si128(m, k[10]); \
    } while (0)

static __m128i aesni128_key_expand(__m128i key, __m128i keygened)
{
    keygened = _mm_shuffle_epi32(keygened, _MM_SHUFFLE(3,3,3,3));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));
    key = _mm_xor_si128(key, _mm_slli_si128(key, 4));

    return _mm_xor_si128(key, keygened);
}

static void aesni128_encrypt_m128i(char *plain_text, char *cipher_text)
{
    __m128i m = _mm_loadu_si128(reinterpret_cast<__m128i *>(plain_text));
    __m128i res; 

    AESNI128_ENC_BLOCK(m, res, key_schedule);

    _mm_storeu_si128(reinterpret_cast<__m128i *>(cipher_text), res);
}

unsigned int aesni128_prng(unsigned int k_n)
{
    unsigned int tmp[4] = {k_n, 0, 0, 0};
    assert(sizeof(unsigned int) == 4);
    aesni128_encrypt_m128i(reinterpret_cast<char *>(tmp), reinterpret_cast<char *>(tmp));
    return tmp[0];
}

void aesni128_load_key(char *enc_key)
{
    key_schedule[0]  = _mm_loadu_si128(reinterpret_cast<const __m128i*>(enc_key));
    key_schedule[1]  = AESNI128_KEY_EXPAND(key_schedule[0], 0x01);
    key_schedule[2]  = AESNI128_KEY_EXPAND(key_schedule[1], 0x02);
    key_schedule[3]  = AESNI128_KEY_EXPAND(key_schedule[2], 0x04);
    key_schedule[4]  = AESNI128_KEY_EXPAND(key_schedule[3], 0x08);
    key_schedule[5]  = AESNI128_KEY_EXPAND(key_schedule[4], 0x10);
    key_schedule[6]  = AESNI128_KEY_EXPAND(key_schedule[5], 0x20);
    key_schedule[7]  = AESNI128_KEY_EXPAND(key_schedule[6], 0x40);
    key_schedule[8]  = AESNI128_KEY_EXPAND(key_schedule[7], 0x80);
    key_schedule[9]  = AESNI128_KEY_EXPAND(key_schedule[8], 0x1B);
    key_schedule[10] = AESNI128_KEY_EXPAND(key_schedule[9], 0x36);
}

void encrypt_int_sum_aesni128(unsigned int *encr_sbuf, const unsigned int *sbuf, int count, int rank,
			      std::vector<unsigned int> &k_s, unsigned int k_n, bool is_edge)
{
    unsigned int tmp1 = k_n + k_s[rank];
    unsigned int tmp2 = k_n + k_s[rank + 1];
    __m128i ind1 = _mm_set_epi32(3 + tmp1, 2 + tmp1, 1 + tmp1, tmp1);
    __m128i ind2 = _mm_set_epi32(3 + tmp2, 2 + tmp2, 1 + tmp2, tmp2);
    __m128i incr = _mm_set_epi32(4, 4, 4, 4);
    __m128i encr_sbuf_vec, noise1, noise2;

    if (!is_edge) {
	for (unsigned int i = 0; i < count; i+=4) {
	    encr_sbuf_vec = _mm_load_si128(reinterpret_cast<const __m128i*>(sbuf + i));
	    AESNI128_ENC_BLOCK(ind1, noise1, key_schedule);
	    AESNI128_ENC_BLOCK(ind2, noise2, key_schedule);
	    encr_sbuf_vec = _mm_add_epi32(encr_sbuf_vec, noise1);
	    encr_sbuf_vec = _mm_sub_epi32(encr_sbuf_vec, noise2);
	    _mm_store_si128(reinterpret_cast<__m128i*>(encr_sbuf + i), encr_sbuf_vec);
	    ind1 = _mm_add_epi32(ind1, incr);
	    ind2 = _mm_add_epi32(ind2, incr);
	}
    } else {
	for (unsigned int i = 0; i < count; i+=4) {
	    encr_sbuf_vec = _mm_load_si128(reinterpret_cast<const __m128i*>(sbuf + i));
	    AESNI128_ENC_BLOCK(ind1, noise1, key_schedule);
	    encr_sbuf_vec = _mm_add_epi32(encr_sbuf_vec, noise1);
	    _mm_store_si128(reinterpret_cast<__m128i*>(encr_sbuf + i), encr_sbuf_vec);
	    ind1 = _mm_add_epi32(ind1, incr);
	}
    }
}

void decrypt_int_sum_aesni128(unsigned int *rbuf, int count, std::vector<unsigned int> &k_s, unsigned int k_n)
{
    unsigned int tmp = k_n + k_s[0];
    __m128i ind  = _mm_set_epi32(3 + tmp, 2 + tmp, 1 + tmp, tmp);
    __m128i incr = _mm_set_epi32(4, 4, 4, 4);
    __m128i decr_rbuf_vec, noise;

    for (unsigned int i = 0; i < count; i+=4) {
	decr_rbuf_vec = _mm_load_si128(reinterpret_cast<const __m128i*>(rbuf + i));
	AESNI128_ENC_BLOCK(ind, noise, key_schedule);
	decr_rbuf_vec = _mm_sub_epi32(decr_rbuf_vec, noise);
	_mm_store_si128(reinterpret_cast<__m128i*>(rbuf + i), decr_rbuf_vec);
	ind = _mm_add_epi32(ind, incr);
    }
}

#endif

}
