#include <vector>
#include <chrono>
#include <iostream>
#include <random>

#include "encrypt.hpp"

using namespace std::chrono;

std::mt19937 random_gen;

const size_t warmup_niters = 25;

template<typename T>
void randomize_vector(std::vector<T> &input)
{
    for (auto &element : input) {
        element = static_cast<T>(random_gen());
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

    std::vector<int> sbuf(nitems, 0);
    std::vector<int> encr_sbuf(nitems, 0);
    std::vector<encr_key_t> k_s(nranks, 0);

    randomize_vector<int>(sbuf);
    randomize_vector<encr_key_t>(k_s);
    encr_key_t k_n = static_cast<encr_key_t>(random_gen());

    std::cout << "Buffer size: " << bufsize << " Bytes" << std::endl;

    for (auto i = 0; i < warmup_niters; i++) {
        __encrypt<int *, int*>(encr_sbuf.data(), sbuf.data(), encr_sbuf.size(),
                               0, k_s, k_n);
    }

    high_resolution_clock::time_point t1_encr = high_resolution_clock::now();

    for (auto i = 0; i < niters; i++) {
        __encrypt<int *, int*>(encr_sbuf.data(), sbuf.data(), encr_sbuf.size(),
                               0, k_s, k_n);
    }

    high_resolution_clock::time_point t2_encr = high_resolution_clock::now();

    duration<double> encr_diff = duration_cast<duration<double>>(t2_encr - t1_encr);
    auto encr_latency = encr_diff.count() / niters;
    auto encr_tput = static_cast<double>(bufsize) / (1e+9) / encr_latency;

    for (auto i = 0; i < warmup_niters; i++) {
        __decrypt<int *>(encr_sbuf.data(), encr_sbuf.size(), k_s, k_n);
    }

    high_resolution_clock::time_point t1_decr = high_resolution_clock::now();

    for (auto i = 0; i < niters; i++) {
        __decrypt<int *>(encr_sbuf.data(), encr_sbuf.size(), k_s, k_n);
    }

    high_resolution_clock::time_point t2_decr = high_resolution_clock::now();

    duration<double> decr_diff = duration_cast<duration<double>>(t2_decr - t1_decr);
    auto decr_latency = decr_diff.count() / niters;
    auto decr_tput = static_cast<double>(bufsize) / (1e+9) / decr_latency;

    std::cout << "Avg encryption time: " << encr_latency << " sec." << std::endl;
    std::cout << "Avg encryption throughput: " << encr_tput << " Gbytes/sec." << std::endl;
    std::cout << "Avg decryption time: " << decr_latency << " sec." << std::endl;
    std::cout << "Avg decryption throughput: " << decr_tput << " Gbytes/sec." << std::endl;

    return 0;
}
