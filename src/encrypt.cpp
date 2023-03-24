#include <cassert>

#include "encrypt.hpp"

namespace encryption {

#ifdef AESNI

__m128i key_schedule[11];

#define AESNI128_KEY_EXPAND(k, rcon) \
    (aesni128_key_expand(k, _mm_aeskeygenassist_si128(k, rcon)))

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

encr_key_t aesni128_prng(encr_key_t k_n)
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

#endif

const encr_key_t max_encr_key = 42; // std::numeric_limits<encr_key_t>::max();
std::random_device rd {};
std::mt19937 encr_key_generator(rd());
std::mt19937 encr_noise_generator;
std::uniform_int_distribution<encr_key_t> encr_key_distr(0, max_encr_key);

encr_key_t generate_encr_key()
{
    return encr_key_distr(encr_key_generator);
}

}
