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

extern const encr_key_t max_encr_key;
extern std::random_device rd;
extern std::mt19937 encr_key_generator;
extern std::mt19937 encr_noise_generator;
extern std::uniform_int_distribution<encr_key_t> encr_key_distr;

encr_key_t generate_encr_key();

inline encr_key_t prng(encr_key_t seed)
{
    encr_noise_generator.seed(seed);
    return encr_key_distr(encr_noise_generator);
    /*
     * Warning:
     * signed integer overflow is UB.
     * TODO: workaround with casting int to uint and do all the stuff safel.
     *
     * 
     *
     */
}

template<typename T, typename K>
void __encrypt_naive(T encr_sbuf, K sbuf, int count, int rank,
		     std::vector<encr_key_t> &k_s, encr_key_t k_n)
{
    unsigned int tmp = static_cast<unsigned int>(k_n + k_s[rank]);

    for (auto i = 0; i < count; i++) {
	encr_sbuf[i] = sbuf[i] + prng(tmp + i);
    }
}

template<typename T>
void __decrypt_naive(T rbuf, int count, std::vector<encr_key_t> &k_s, encr_key_t k_n)
{
    unsigned int key;

    for (auto j = 0; j < k_s.size(); j++) {
	key = static_cast<unsigned int>(k_s[j] + k_n);
	for (auto i = 0; i < count; i++) {
            rbuf[i] = rbuf[i] - prng(key + i);
        }
    }
}

template<typename T, typename K>
void __encrypt2_naive(T encr_sbuf, K sbuf, int count, int rank,
		      std::vector<encr_key_t> &k_s, encr_key_t k_n, bool is_edge)
{
    unsigned int tmp1 = static_cast<unsigned int>(k_n + k_s[rank]);
    unsigned int tmp2 = static_cast<unsigned int>(k_n + k_s[rank + 1]);

    if (!is_edge) {
	for (auto i = 0; i < count; i++) {
	    encr_sbuf[i] = sbuf[i] + prng(tmp1 + i) - prng(tmp2 + i);
	}
    } else {
	for (auto i = 0; i < count; i++) {
	    encr_sbuf[i] = sbuf[i] + prng(tmp1 + i);
	}
    }
}

template<typename T>
void __decrypt2_naive(T rbuf, int count, std::vector<encr_key_t> &k_s, encr_key_t k_n)
{
    unsigned int tmp = static_cast<unsigned int>(k_n + k_s[0]);

    for (auto i = 0; i < count; i++) {
	rbuf[i] = rbuf[i] - prng(k_n + k_s[0] + i);
    }
}

#ifdef AESNI

extern __m128i key_schedule[11];
encr_key_t aesni128_prng(encr_key_t);
void aesni128_load_key(char *enc_key);

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

template<typename T, typename K>
void __encrypt2_aesni128(T encr_sbuf, K sbuf, int count, int rank,
			std::vector<encr_key_t> &k_s, encr_key_t k_n, bool is_edge)
{
    unsigned int tmp1 = static_cast<unsigned int>(k_n + k_s[rank]);
    unsigned int tmp2 = static_cast<unsigned int>(k_n + k_s[rank + 1]);
    __m128i ind1 = _mm_set_epi32(3 + tmp1, 2 + tmp1, 1 + tmp1, tmp1);
    __m128i ind2 = _mm_set_epi32(3 + tmp2, 2 + tmp2, 1 + tmp2, tmp2);
    __m128i incr = _mm_set_epi32(4, 4, 4, 4);
    __m128i encr_sbuf_vec, noise1, noise2;

    if (!is_edge) {
	for (auto i = 0; i < count; i+=4) {
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
	for (auto i = 0; i < count; i+=4) {
	    encr_sbuf_vec = _mm_load_si128(reinterpret_cast<const __m128i*>(sbuf + i));
	    AESNI128_ENC_BLOCK(ind1, noise1, key_schedule);
	    encr_sbuf_vec = _mm_add_epi32(encr_sbuf_vec, noise1);
	    _mm_store_si128(reinterpret_cast<__m128i*>(encr_sbuf + i), encr_sbuf_vec);
	    ind1 = _mm_add_epi32(ind1, incr);
	}
    }
}

template<typename T>
void __decrypt2_aesni128(T rbuf, int count, std::vector<encr_key_t> &k_s, encr_key_t k_n)
{
    unsigned int tmp = static_cast<unsigned int>(k_n + k_s[0]);
    __m128i ind  = _mm_set_epi32(3 + tmp, 2 + tmp, 1 + tmp, tmp);
    __m128i incr = _mm_set_epi32(4, 4, 4, 4);
    __m128i decr_rbuf_vec, noise;

    for (auto i = 0; i < count; i+=4) {
	decr_rbuf_vec = _mm_load_si128(reinterpret_cast<const __m128i*>(rbuf + i));
	AESNI128_ENC_BLOCK(ind, noise, key_schedule);
	decr_rbuf_vec = _mm_sub_epi32(decr_rbuf_vec, noise);
	_mm_store_si128(reinterpret_cast<__m128i*>(rbuf + i), decr_rbuf_vec);
	ind = _mm_add_epi32(ind, incr);
    }
}

#endif

}

#endif
