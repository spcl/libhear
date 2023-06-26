#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <random>
#include <limits>
#include <fstream>

#define EXPERIMENT_STEP_SIZE 10000
#define EXPERIMENT_NUMBER_OF_STEPS 10000

int main() {
  std::random_device rd;
  std::mt19937_64 eng(rd());
  std::uniform_int_distribution<unsigned long long> distr;

  for (int i = 0; i < EXPERIMENT_NUMBER_OF_STEPS; i++) {
    unsigned long long noise_offset = distr(eng);
    unsigned long long sum_encrypted = 0;
    unsigned long long sum_original = 0;
    unsigned long long sum_noise = 0;
    for (int j = 0; j < EXPERIMENT_STEP_SIZE; j++) {
      unsigned long long original_number = distr(eng);
      unsigned long long noise_number = distr(eng) + noise_offset;
      sum_original += original_number;
      sum_noise += noise_number;
      sum_encrypted += noise_number + original_number;
    }
    assert(sum_encrypted - sum_noise - sum_original == 0);
  }
  return 0;
}