#include <iomanip>
#include <vector>
#include <random>
#include <chrono>

#include "hfloat.hpp"

#define SEED 42

int main()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    gen.seed(SEED);
    std::uniform_int_distribution<uint32_t> idist(0, std::numeric_limits<uint32_t>::max());
    std::uniform_real_distribution<float> fdist(0, 1);

    for (auto i = 0; i < TEST_SIZE; i++) {
	uint32_t noise = idist(gen);
	std::vector<float> x(TEST_STEP_SIZE);
	for (auto &f : x)
	    f = fdist(gen);

	auto start = std::chrono::high_resolution_clock::now();
	for (int j = 0; j < TEST_STEP_SIZE; j++) {
	    uint32_t n = noise;
	    HNumbers::HFloat c = reinterpret_cast<HNumbers::HFloat &>(noise);
#ifdef DEBUG
	    std::cout << "Original float: " << std::endl;
	    HNumbers::cout_float(x[j]);
	    float initial_x = x[j];
#endif
#ifdef DEBUG
	    std::cout << "Noise float: " << std::endl;
	    std::cout << c << std::endl;
#endif
	    c.encrypt(x[j]);
#ifdef DEBUG
	    std::cout << "Encrypted float: " << std::endl;
	    std::cout << c << std::endl;
#endif
	    c.decrypt(x[j], noise);
#ifdef DEBUG
	    std::cout << "Decrypted float: " << std::endl;
	    HNumbers::cout_float(x[j]);
	    std::cout << "Stats: " << std::endl;
	    std::cout << initial_x << " " << x[j] << " " << (initial_x - x[j]) << std::endl << std::endl;
#endif
	}

	auto end = std::chrono::high_resolution_clock::now();
	auto duration = end - start;
	auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration); // Microsecond (as int)
	auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration); // Milliseconds (as int)
	const float ms_fractional = static_cast<float>(us.count()) / 1000.0;       // Milliseconds (as float)
	std::cout << "Duration = " << us.count() << "µs (" << ms_fractional << "ms)" << std::endl;
    }

    return 0;
}
