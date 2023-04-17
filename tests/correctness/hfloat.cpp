#include <iomanip>
#include <vector>
#include <random>
#include <chrono>

#include "hfloat.hpp"

#define SEED 42

int main() {
  std::random_device rd;
  std::mt19937 gen(rd());
  gen.seed(42);
  std::uniform_int_distribution<uint32_t> idist(0, std::numeric_limits<uint32_t>::max());
  std::uniform_real_distribution<float> fdist(0, 1);
  double ttotal_noise = 0;
  for (int i = 0; i < TEST_SIZE; i++) {
    uint32_t noise = idist(gen);
    std::vector<float> x(TEST_STEP_SIZE);
    for (auto &f : x) {
      f = fdist(gen);
    }

    auto start = std::chrono::high_resolution_clock::now();
    double total_noise = 0;
    for (int j = 0; j < TEST_STEP_SIZE; j++) {
      uint32_t n = noise;
      HNumbers::HFloat c = reinterpret_cast<HNumbers::HFloat &>(noise);
      float initial_x = x[j];
      #ifdef DEBUG
      std::cout << "Original float: " << std::endl;
      cout_float(x[j]);
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
      total_noise += (initial_x - x[j])/initial_x;
      #ifdef DEBUG
      std::cout << "Decrypted float: " << std::endl;
      cout_float(x[j]);
      std::cout << "Stats: " << std::endl;
      std::cout << initial_x << " " << x[j] << " " << (initial_x - x[j]) << std::endl << std::endl;
      #endif
    }
    ttotal_noise += total_noise;
    std::cout << "Error: " << total_noise/TEST_STEP_SIZE << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = (end - start);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration); // Microsecond (as int)
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration); // Milliseconds (as int)
    const float ms_fractional = static_cast<float>(us.count()) / 1000;         // Milliseconds (as float)
    std::cout << "Duration: " << us.count() << "µs (" << ms_fractional << "ms)" << std::endl << std::endl;
  }
  std::cout << "Total average error: " << ttotal_noise/TEST_STEP_SIZE/TEST_SIZE << std::endl;
  return 0;
}