#include <cassert>

#include "encrypt.hpp"
#include "hfloat.hpp"

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
    signed int exponent;
    HNumbers::HNumber hnum;

    for (unsigned int i = 0; i < count; i++) {
	hnum.native_float = prng_uint(k_n + i);
	exponent = hnum.crypto.exponent;
	hnum.ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
	hnum.ieee_float.ieee.mantissa <<= SHIFT;
	hnum.native_float *= sbuf[i];
	hnum.crypto_simplified.remainder >>= SHIFT;
	hnum.crypto.exponent += exponent - IEEE754_FLOAT_BIAS;
	encr_sbuf[i] = hnum.native_float;
    }
}

void decrypt_float_sum_naive(float *rbuf, int count, std::vector<unsigned int> &k_s, unsigned int k_n)
{
    HNumbers::HNumber hnum;
    HNumbers::HNumber noise;

    for (unsigned int i = 0; i < count; i++) {
	noise.native_float = prng_uint(k_n + i);
	hnum = reinterpret_cast<HNumbers::HNumber &>(rbuf[i]);
	hnum.crypto.exponent -= noise.crypto.exponent;
	hnum.crypto.exponent += IEEE754_FLOAT_BIAS;
	hnum.crypto.exponent <<= SHIFT;
	hnum.ieee_float.ieee.mantissa <<= SHIFT;
	noise.ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
	noise.ieee_float.ieee.mantissa <<= SHIFT;
	rbuf[i] = hnum.native_float / noise.native_float;
    }
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

void encrypt_int_sum_aesni128_unroll(unsigned int * __restrict__ encr_sbuf, const unsigned int * __restrict__ sbuf,
				     int count, int rank, std::vector<unsigned int> &k_s, unsigned int k_n, bool is_edge)
{
    unsigned int tmp1 = k_n + k_s[rank];
    unsigned int tmp2 = k_n + k_s[rank + 1];
    unsigned int ind1[4] = {tmp1, tmp1 + 1, tmp1 + 2, tmp1 + 3};
    unsigned int ind2[4] = {tmp2, tmp2 + 1, tmp2 + 2, tmp2 + 3};
    unsigned int noise1[4];
    unsigned int noise2[4];

    encr_sbuf = (unsigned int *)__builtin_assume_aligned(encr_sbuf, 32);
    sbuf = (const unsigned int *)__builtin_assume_aligned(sbuf, 32);

    if (!is_edge) {
	for (unsigned int i = 0; i < count; i+=4) {
	    AESNI128_ENC_BLOCK(*((__m128i *)ind1), *((__m128i *)noise1), key_schedule);
	    AESNI128_ENC_BLOCK(*((__m128i *)ind2), *((__m128i *)noise2), key_schedule);

	    noise1[0] = noise1[0] - noise2[0];
            noise1[1] = noise1[1] - noise2[1];
            noise1[2] = noise1[2] - noise2[2];
            noise1[3] = noise1[3] - noise2[3];

	    encr_sbuf[i] = sbuf[i];
            encr_sbuf[i + 1] = sbuf[i + 1];
            encr_sbuf[i + 2] = sbuf[i + 2];
            encr_sbuf[i + 3] = sbuf[i + 3];

	    encr_sbuf[i] += noise1[0];
            encr_sbuf[i + 1] += noise1[1];
            encr_sbuf[i + 2] += noise1[2];
            encr_sbuf[i + 3] +=  noise1[3];

	    ind1[0] += 4;
            ind1[1] += 4;
            ind1[2] += 4;
            ind1[3] += 4;

	    ind2[0] += 4;
            ind2[1] += 4;
            ind2[2] += 4;
            ind2[3] += 4;
	}
    } else {
	for (unsigned int i = 0; i < count; i+=4) {
	    AESNI128_ENC_BLOCK(*((__m128i *)ind1), *((__m128i *)noise1), key_schedule);

	    encr_sbuf[i] = sbuf[i];
	    encr_sbuf[i + 1] = sbuf[i + 1];
	    encr_sbuf[i + 2] = sbuf[i + 2];
	    encr_sbuf[i + 3] = sbuf[i + 3];

	    encr_sbuf[i] += noise1[0];
            encr_sbuf[i + 1] += noise1[1];
            encr_sbuf[i + 2] += noise1[2];
            encr_sbuf[i + 3] +=  noise1[3];

	    ind1[0] += 4;
	    ind1[1] += 4;
	    ind1[2] += 4;
	    ind1[3] += 4;
	}
    }
}

void decrypt_int_sum_aesni128_unroll(unsigned int * __restrict__ rbuf, int count,
				     std::vector<unsigned int> &k_s, unsigned int k_n)
{
    unsigned int tmp = k_n + k_s[0];
    unsigned int ind[4] = {tmp, tmp + 1, tmp + 2, tmp + 3};
    unsigned int noise[4];

    rbuf = (unsigned int *)__builtin_assume_aligned(rbuf, 32);

    for (unsigned int i = 0; i < count; i+=4) {
	AESNI128_ENC_BLOCK(*((__m128i *)ind), *((__m128i *)noise), key_schedule);

	rbuf[i] = rbuf[i] - noise[0];
	rbuf[i + 1] = rbuf[i + 1] - noise[1];
	rbuf[i + 2] = rbuf[i + 2] - noise[2];
	rbuf[i + 3] = rbuf[i + 3] - noise[3];

	ind[0] += 4;
	ind[1] += 4;
	ind[2] += 4;
	ind[3] += 4;
    }
}

void encrypt_float_sum_aesni128_unroll(float * __restrict__ encr_sbuf, const float * __restrict__ sbuf,
				       int count, int rank, std::vector<unsigned int> &k_s, unsigned int k_n)
{
    unsigned int ind[4] = {k_n + 1, k_n + 2, k_n + 3, k_n + 4};
    signed int exponent[4];
    HNumbers::HNumber hnum[4];

    encr_sbuf = (float *)__builtin_assume_aligned(encr_sbuf, 32);
    sbuf = (const float *)__builtin_assume_aligned(sbuf, 32);

    for (unsigned int i = 0; i < count; i+=4) {
        AESNI128_ENC_BLOCK(*((__m128i *)ind), *((__m128i *)hnum), key_schedule);

	exponent[0] = hnum[0].crypto.exponent;
	exponent[1] = hnum[1].crypto.exponent;
	exponent[2] = hnum[2].crypto.exponent;
	exponent[3] = hnum[3].crypto.exponent;

	hnum[0].ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
	hnum[1].ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
	hnum[2].ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
	hnum[3].ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;

	hnum[0].ieee_float.ieee.mantissa <<= SHIFT;
	hnum[1].ieee_float.ieee.mantissa <<= SHIFT;
	hnum[2].ieee_float.ieee.mantissa <<= SHIFT;
	hnum[3].ieee_float.ieee.mantissa <<= SHIFT;

	hnum[0].native_float *= sbuf[i];
	hnum[1].native_float *= sbuf[i + 1];
	hnum[2].native_float *= sbuf[i + 2];
	hnum[3].native_float *= sbuf[i + 3];

	hnum[0].crypto_simplified.remainder >>= SHIFT;
	hnum[1].crypto_simplified.remainder >>= SHIFT;
	hnum[2].crypto_simplified.remainder >>= SHIFT;
	hnum[3].crypto_simplified.remainder >>= SHIFT;

	hnum[0].crypto.exponent += exponent[0] - IEEE754_FLOAT_BIAS;
	hnum[1].crypto.exponent += exponent[1] - IEEE754_FLOAT_BIAS;
	hnum[2].crypto.exponent += exponent[2] - IEEE754_FLOAT_BIAS;
	hnum[3].crypto.exponent += exponent[3] - IEEE754_FLOAT_BIAS;

	encr_sbuf[i] = hnum[0].native_float;
	encr_sbuf[i + 1] = hnum[1].native_float;
	encr_sbuf[i + 2] = hnum[2].native_float;
	encr_sbuf[i + 3] = hnum[3].native_float;

	ind[0] += 4;
	ind[1] += 4;
	ind[2] += 4;
	ind[3] += 4;
    }
}

void decrypt_float_sum_aesni128_unroll(float * __restrict__ rbuf, int count,
				       std::vector<unsigned int> &k_s, unsigned int k_n)
{
    unsigned int ind[4] = {k_n + 1, k_n + 2, k_n + 3, k_n + 4};
    HNumbers::HNumber hnum[4];
    HNumbers::HNumber noise[4];

    rbuf = (float *)__builtin_assume_aligned(rbuf, 32);

    for (unsigned int i = 0; i < count; i+=4) {
        AESNI128_ENC_BLOCK(*((__m128i *)ind), *((__m128i *)noise), key_schedule);

	hnum[0] = reinterpret_cast<HNumbers::HNumber &>(rbuf[i + 0]);
	hnum[1] = reinterpret_cast<HNumbers::HNumber &>(rbuf[i + 1]);
	hnum[2] = reinterpret_cast<HNumbers::HNumber &>(rbuf[i + 2]);
	hnum[3] = reinterpret_cast<HNumbers::HNumber &>(rbuf[i + 3]);

	hnum[0].crypto.exponent -= noise[0].crypto.exponent;
	hnum[1].crypto.exponent -= noise[1].crypto.exponent;
	hnum[2].crypto.exponent -= noise[2].crypto.exponent;
	hnum[3].crypto.exponent -= noise[3].crypto.exponent;

	hnum[0].crypto.exponent += IEEE754_FLOAT_BIAS;
	hnum[1].crypto.exponent += IEEE754_FLOAT_BIAS;
	hnum[2].crypto.exponent += IEEE754_FLOAT_BIAS;
	hnum[3].crypto.exponent += IEEE754_FLOAT_BIAS;

	hnum[0].crypto.exponent <<= SHIFT;
	hnum[1].crypto.exponent <<= SHIFT;
	hnum[2].crypto.exponent <<= SHIFT;
	hnum[3].crypto.exponent <<= SHIFT;

	hnum[0].ieee_float.ieee.mantissa <<= SHIFT;
	hnum[1].ieee_float.ieee.mantissa <<= SHIFT;
	hnum[2].ieee_float.ieee.mantissa <<= SHIFT;
	hnum[3].ieee_float.ieee.mantissa <<= SHIFT;

	noise[0].ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
	noise[1].ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
	noise[2].ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
	noise[3].ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;

	noise[0].ieee_float.ieee.mantissa <<= SHIFT;
	noise[1].ieee_float.ieee.mantissa <<= SHIFT;
	noise[2].ieee_float.ieee.mantissa <<= SHIFT;
	noise[3].ieee_float.ieee.mantissa <<= SHIFT;

	rbuf[i]     = hnum[0].native_float / noise[0].native_float;
	rbuf[i + 1] = hnum[1].native_float / noise[1].native_float;
	rbuf[i + 2] = hnum[2].native_float / noise[2].native_float;
	rbuf[i + 3] = hnum[3].native_float / noise[3].native_float;

	ind[0] += 4;
	ind[1] += 4;
	ind[2] += 4;
	ind[3] += 4;
    }
}

#endif

}
