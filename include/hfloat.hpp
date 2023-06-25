#ifndef _HFLOAT_HPP_
#define _HFLOAT_HPP_

#include <cstdint>
#include <iostream>
#include <iomanip>
#include <ieee754.h>
#include <bitset>

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

namespace HNumbers
{

union HNumber
{

    ieee754_float ieee_float;
    float native_float;

    struct
    {
	unsigned int mantissa : FLOAT_MANTISSA;
	signed int exponent : FLOAT_EXPONENT;
	unsigned int sign : 1;
    } crypto;

    struct
    {
	unsigned int remainder : FLOAT_MANTISSA + FLOAT_EXPONENT;
	unsigned int sign : 1;
    } crypto_simplified;

};

class HFloat
{
private:

    HNumber _number;

    void encrypt(const float &num);
    void decrypt(float &num, uint32_t noise);
        
    friend std::ostream& operator<<(std::ostream& os, const HFloat& h_float)
    {
	os << h_float._number.crypto.sign << " ";
	os << std::setw(FLOAT_EXPONENT) << h_float._number.crypto.exponent << " ";
	os << std::setw(FLOAT_MANTISSA) << h_float._number.crypto.mantissa << std::endl;
	os << std::bitset<1>(h_float._number.crypto.sign) << " ";
	os << std::bitset<FLOAT_EXPONENT>(h_float._number.crypto.exponent) << " ";
	os << std::bitset<FLOAT_MANTISSA>(h_float._number.crypto.mantissa);
	return os;
    }

public:
    HNumber& number() { return _number; }

};

void cout_float(const float f);

};

#endif
