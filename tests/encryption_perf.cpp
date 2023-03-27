#include <vector>
#include <chrono>
#include <iostream>
#include <random>

#include "encrypt.hpp"

using namespace std::chrono;

std::mt19937 random_gen;

const size_t warmup_niters = 25;

template<typename T>
void randomize_vector(T *input, std::size_t count)
{
    for (auto i = 0; i < count; i++) {
        input[i] = static_cast<T>(random_gen());
    }
}

template<typename T>
void print_vector(T *input, std::size_t count)
{
    for (auto i = 0; i < count; i++) {
	std::cout << input[i] << std::endl;
    }
}

int main(int argc, char **argv)
{
    if (argc != 4) {
        std::cerr << "Bad arguments!" << std::endl;
        exit(EXIT_FAILURE);
    }

    std::size_t niters = std::atoi(argv[1]);
    std::size_t nitems = std::atoi(argv[2]);
    std::size_t nranks = std::atoi(argv[3]);
    std::size_t bufsize = nitems * sizeof(int);

#ifdef AESNI
    __m256i *sbuf = reinterpret_cast<__m256i*>(_mm_malloc(bufsize, 32));
    __m256i *encr_sbuf = reinterpret_cast<__m256i*>(_mm_malloc(bufsize, 32));
#else
    unsigned int *sbuf = new unsigned int[nitems];
    unsigned int *encr_sbuf = new unsigned int[nitems];
#endif
    std::vector<unsigned int> k_s(nranks);

    random_gen.seed(42);
    randomize_vector<unsigned int>(reinterpret_cast<unsigned int *>(sbuf), nitems);
    randomize_vector<unsigned int>(k_s.data(), nranks);
    unsigned int k_n = static_cast<unsigned int>(random_gen());

#ifdef AESNI
    char encr_key[] = {0x2b, 0x7e, 0x15, 0x16,
		       0x28, 0xae, 0xd2, 0xa6,
		       0xab, 0xf7, 0x15, 0x88,
		       0x09, 0xcf, 0x4f, 0x3c};
    encryption::aesni128_load_key(encr_key);
#endif
    
    std::cout << "Buffer size: " << bufsize << " Bytes" << std::endl;

    for (auto i = 0; i < warmup_niters; i++) {
#ifdef AESNI
	encryption::encrypt_int_sum_aesni128(reinterpret_cast<unsigned int *>(encr_sbuf),
					     reinterpret_cast<unsigned int *>(sbuf),
					     nitems, 0, k_s, k_n, 0);
#else
	encryption::encrypt_int_sum_naive(reinterpret_cast<unsigned int *>(encr_sbuf),
					  reinterpret_cast<unsigned int *>(sbuf),
					  nitems, 0, k_s, k_n, 0);
#endif
    }

    high_resolution_clock::time_point t1_encr = high_resolution_clock::now();

    for (auto i = 0; i < niters; i++) {
#ifdef AESNI
	encryption::encrypt_int_sum_aesni128(reinterpret_cast<unsigned int *>(encr_sbuf),
					     reinterpret_cast<unsigned int *>(sbuf),
					     nitems, 0, k_s, k_n, 0);
#else
	encryption::encrypt_int_sum_naive(reinterpret_cast<unsigned int *>(encr_sbuf),
					  reinterpret_cast<unsigned int *>(sbuf),
					  nitems, 0, k_s, k_n, 0);
#endif
    }

    high_resolution_clock::time_point t2_encr = high_resolution_clock::now();

    for (auto i = 0; i < warmup_niters; i++) {
#ifdef AESNI
	encryption::decrypt_int_sum_aesni128(reinterpret_cast<unsigned int *>(encr_sbuf), nitems, k_s, k_n);
#else
	encryption::decrypt_int_sum_naive(reinterpret_cast<unsigned int *>(encr_sbuf), nitems, k_s, k_n);
#endif
    }

    high_resolution_clock::time_point t1_decr = high_resolution_clock::now();

    for (auto i = 0; i < niters; i++) {
#ifdef AESNI
	encryption::decrypt_int_sum_aesni128(reinterpret_cast<unsigned int *>(encr_sbuf), nitems, k_s, k_n);
#else
	encryption::decrypt_int_sum_naive(reinterpret_cast<unsigned int *>(encr_sbuf), nitems, k_s, k_n);
#endif
    }

    high_resolution_clock::time_point t2_decr = high_resolution_clock::now();

    duration<double> encr_diff = duration_cast<duration<double>>(t2_encr - t1_encr);
    auto encr_latency = encr_diff.count() / niters;
    auto encr_tput = static_cast<double>(bufsize) / (1e+9) / encr_latency;
    duration<double> decr_diff = duration_cast<duration<double>>(t2_decr - t1_decr);
    auto decr_latency = decr_diff.count() / niters;
    auto decr_tput = static_cast<double>(bufsize) / (1e+9) / decr_latency;
    std::cout << "Avg encryption time: " << encr_latency << " sec." << std::endl;
    std::cout << "Avg encryption throughput: " << encr_tput << " Gbytes/sec." << std::endl;
    std::cout << "Avg decryption time: " << decr_latency << " sec." << std::endl;
    std::cout << "Avg decryption throughput: " << decr_tput << " Gbytes/sec." << std::endl;

#ifdef AESNI
    _mm_free(sbuf);
    _mm_free(encr_sbuf);
#else
    delete[] sbuf;
    delete[] encr_sbuf;
#endif

    return 0;
}
