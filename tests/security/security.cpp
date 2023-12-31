#include "hfloat.hpp"
#include <vector>
#include <fstream>
#include <unordered_map>
#include <string>
#include <mpi.h>

int power2(int a) {
  int result = 1;
  for (int i = 0; i < a; i++) {
    result *= 2;
  }
  return result;
}

std::vector<uint64_t> compute(int option1, int option2) {
  std::vector<uint64_t> max_mantissas(power2(FLOAT_MANTISSA), 0);
  std::vector<uint64_t> max_mantissa_indexes(power2(FLOAT_MANTISSA), 0);

  HNumbers::HFloat x[4];
  HNumbers::HFloat n[4];
  for (int i = 0; i < 4; i++) {
    x[i].number().ieee_float.ieee.negative = 0;
    x[i].number().ieee_float.ieee.exponent = IEEE_FLOAT_EXPONENT;
    x[i].number().ieee_float.ieee.mantissa = 42;
  }
  uint64_t power = power2(FLOAT_MANTISSA);
  for (int j = option1; j < option2; j++) {
    std::vector<uint64_t> mantissas(power2(FLOAT_MANTISSA), 0);
    if (option1 == 0 and j % 10 == 0)
        std::cout << "Rank 0 processing iteration " << j << " out of " << option2 << std::endl;
    for (uint64_t i = 0; i < power; i+= 4) {
      n[0].number().crypto.sign = 0;
      n[0].number().crypto.exponent = 0;
      n[0].number().crypto.mantissa = i;
      n[1].number().crypto.sign = 0;
      n[1].number().crypto.exponent = 0;
      n[1].number().crypto.mantissa = i + 1;
      n[2].number().crypto.sign = 0;
      n[2].number().crypto.exponent = 0;
      n[2].number().crypto.mantissa = i + 2;
      n[3].number().crypto.sign = 0;
      n[3].number().crypto.exponent = 0;
      n[3].number().crypto.mantissa = i + 3;

      x[0].number().ieee_float.ieee.mantissa = j;
      x[1].number().ieee_float.ieee.mantissa = j;
      x[2].number().ieee_float.ieee.mantissa = j;
      x[3].number().ieee_float.ieee.mantissa = j;

      n[0].encrypt(x[0].number().native_float);
      n[1].encrypt(x[1].number().native_float);
      n[2].encrypt(x[2].number().native_float);
      n[3].encrypt(x[3].number().native_float);

      mantissas[n[0].number().crypto.mantissa] += 1;
      mantissas[n[1].number().crypto.mantissa] += 1;
      mantissas[n[2].number().crypto.mantissa] += 1;
      mantissas[n[3].number().crypto.mantissa] += 1;
    }
      int64_t previous_value = mantissas[0];
      int64_t count = 0;
      for (uint64_t i = 0; i < power; i++) {
        if (mantissas[i] > max_mantissas[i]) {
          max_mantissas[i] = mantissas[i];
          max_mantissa_indexes[i] = j;
        }
      }

  }

  std::string overall_filename = "tests/security/mantissas_overall_" + std::to_string(option1) + "-" + std::to_string(option2) + ".csv";
  std::ofstream overall_file(overall_filename);
  if (overall_file) {
    overall_file << "Index,Key,Value\n";
    for (uint64_t i = 0; i < power; i++) {
      overall_file << max_mantissa_indexes[i] << "," << i << "," << max_mantissas[i] << "\n";
    }
  }
  overall_file.close();

  return max_mantissas;
}

int main() {
      // Initialize the MPI environment
    MPI_Init(NULL, NULL);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    int NUM_TASKS=8388608;
    int NUM_CORES=95;

    float TASKS_PER_CORE = (float)NUM_TASKS / NUM_CORES;
    std::vector<uint64_t> results;
    if (world_rank == NUM_CORES) {
      std::cout << "Rank " << world_rank << " processing " << (world_rank+1) * TASKS_PER_CORE << "-" << NUM_TASKS << std::endl;
      results = compute((world_rank+1) * TASKS_PER_CORE, NUM_TASKS);
    } else {
      int START_TASK=world_rank * TASKS_PER_CORE;
      int END_TASK=(world_rank+1) * TASKS_PER_CORE;
      std::cout << "Rank " << world_rank << " processing " << START_TASK << "-" << END_TASK << std::endl;
      results = compute(START_TASK, END_TASK);
    }

    std::vector<uint64_t> overall_results(results.size(), 0);
    MPI_Reduce(results.data(), overall_results.data(), results.size(), MPI_UNSIGNED_LONG, MPI_MAX, 0, MPI_COMM_WORLD);
    if (world_rank == 0) {
      std::string overall_filename = "tests/security/mantissas_total.csv";
      std::ofstream overall_file(overall_filename);
      if (overall_file) {
        overall_file << "Index,Key,Value\n";
        for (uint64_t i = 0; i < overall_results.size(); i++) {
          overall_file << i << "," << overall_results[i] << "\n";
        }
      }
      overall_file.close();
    }

    // Finalize the MPI environment.
    MPI_Finalize();
}
