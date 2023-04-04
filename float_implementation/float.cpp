#include <cstdint>
#include <iostream>
#include <ieee754.h>
#include <iomanip>
#include <bitset>
#include <vector>
#include <random>
#include <chrono>

#define TEST_SIZE 10
#define TEST_STEP_SIZE 1000000
#define FLOAT_MANTISSA 21
#define FLOAT_EXPONENT 10
#define DOUBLE_MANTISSA 52
#define DOUBLE_EXPONENT 11
#define IEEE_FLOAT_MANTISSA 23
#define IEEE_FLOAT_EXPONENT 8
#define IEEE_DOUBLE_MANTISSA 52
#define IEEE_DOUBLE_EXPONENT 11
#define SHIFT (FLOAT_EXPONENT - IEEE_FLOAT_EXPONENT)


union HNumber {
  ieee754_float ieee_float;
  float native_float;
  struct {
    unsigned int mantissa : FLOAT_MANTISSA;
    signed int exponent : FLOAT_EXPONENT;
    unsigned int sign : 1;
  } crypto;
  struct {
    unsigned int remainder : FLOAT_MANTISSA + FLOAT_EXPONENT;
    unsigned int sign : 1;
  } crypto_simplified;
};

class h_float {
  private:
    HNumber number;
  public:
    void encrypt(const float &num) {
      signed int exponent = number.crypto.exponent;
      number.ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
      number.ieee_float.ieee.mantissa <<= SHIFT;
      number.native_float *= num;
      number.crypto_simplified.remainder >>= SHIFT;
      number.crypto.exponent += exponent - IEEE754_FLOAT_BIAS;
    }

    void decrypt(float &num, uint32_t noise) {
      h_float h_noise = reinterpret_cast<h_float &>(noise);
      number.crypto.exponent -= h_noise.number.crypto.exponent;
      number.crypto.exponent += IEEE754_FLOAT_BIAS;
      number.crypto.exponent <<= SHIFT;
      number.ieee_float.ieee.mantissa <<= SHIFT;
      h_noise.number.ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
      h_noise.number.ieee_float.ieee.mantissa <<= SHIFT;
      num = number.native_float / h_noise.number.native_float;
    }

    friend std::ostream& operator<<(std::ostream& os, const h_float& h_float) {
      os << h_float.number.crypto.sign << " ";
      os << std::setw(FLOAT_EXPONENT) << h_float.number.crypto.exponent << " ";
      os << std::setw(FLOAT_MANTISSA) << h_float.number.crypto.mantissa << std::endl;
      os << std::bitset<1>(h_float.number.crypto.sign) << " ";
      os << std::bitset<FLOAT_EXPONENT>(h_float.number.crypto.exponent) << " ";
      os << std::bitset<FLOAT_MANTISSA>(h_float.number.crypto.mantissa);
      return os;
    }
};

void cout_float(const float f) {
  ieee754_float x = reinterpret_cast<const ieee754_float &>(f);
  std::cout << x.ieee.negative << " ";
  std::cout << std::setw(IEEE_FLOAT_EXPONENT) << x.ieee.exponent << " ";
  std::cout << std::setw(IEEE_FLOAT_MANTISSA) << x.ieee.mantissa << std::endl;
  std::cout << std::bitset<1>(x.ieee.negative) << " ";
  std::cout << std::bitset<IEEE_FLOAT_EXPONENT>(x.ieee.exponent) << " ";
  std::cout << std::bitset<IEEE_FLOAT_MANTISSA>(x.ieee.mantissa) << std::endl;
}

int main() {
  std::random_device rd;
  std::mt19937 gen(rd());
  gen.seed(42);
  std::uniform_int_distribution<uint32_t> idist(0, std::numeric_limits<uint32_t>::max());
  std::uniform_real_distribution<float> fdist(0, 1);

  for (int i = 0; i < TEST_SIZE; i++) {
    uint32_t noise = idist(gen);
    std::vector<float> x(TEST_STEP_SIZE);
    for (auto &f : x) {
      f = fdist(gen);
    }

    auto start = std::chrono::high_resolution_clock::now();
    for (int j = 0; j < TEST_STEP_SIZE; j++) {
      uint32_t n = noise;
      h_float c = reinterpret_cast<h_float &>(noise);
      #ifdef DEBUG
        cout_float(x[j]);
        float initial_x = x[j];
      #endif
      #ifdef DEBUG
        std::cout << c << std::endl;
      #endif
      c.encrypt(x[j]);
      #ifdef DEBUG
        std::cout << c << std::endl;
        std::cout << noise << " " << x[j] << " ";
      #endif
      c.decrypt(x[j], noise);
      #ifdef DEBUG
        std::cout << x[j] << " " << (initial_x - x[j]) << std::endl;
      #endif
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = (end - start);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration); // Microsecond (as int)
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration); // Milliseconds (as int)
    const float ms_fractional = static_cast<float>(us.count()) / 1000;         // Milliseconds (as float)
    std::cout << "Duration = " << us.count() << "µs (" << ms_fractional << "ms)" << std::endl;
  }
  return 0;
}