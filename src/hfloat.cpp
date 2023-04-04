#include "hfloat.hpp"

namespace HNumbers
{

void HFloat::encrypt(const float &num)
{
    signed int exponent = _number.crypto.exponent;
    _number.ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
    _number.ieee_float.ieee.mantissa <<= SHIFT;
    _number.native_float *= num;
    _number.crypto_simplified.remainder >>= SHIFT;
    _number.crypto.exponent += exponent - IEEE754_FLOAT_BIAS;
}

void HFloat::decrypt(float &num, uint32_t noise)
{
    HFloat h_noise = reinterpret_cast<HFloat &>(noise);
    _number.crypto.exponent -= h_noise._number.crypto.exponent;
    _number.crypto.exponent += IEEE754_FLOAT_BIAS;
    _number.crypto.exponent <<= SHIFT;
    _number.ieee_float.ieee.mantissa <<= SHIFT;
    h_noise._number.ieee_float.ieee.exponent = IEEE754_FLOAT_BIAS;
    h_noise._number.ieee_float.ieee.mantissa <<= SHIFT;
    num = _number.native_float / h_noise._number.native_float;
}

void cout_float(const float f)
{
    ieee754_float x = reinterpret_cast<const ieee754_float &>(f);
    std::cout << x.ieee.negative << " ";
    std::cout << std::setw(IEEE_FLOAT_EXPONENT) << x.ieee.exponent << " ";
    std::cout << std::setw(IEEE_FLOAT_MANTISSA) << x.ieee.mantissa << std::endl;
    std::cout << std::bitset<1>(x.ieee.negative) << " ";
    std::cout << std::bitset<IEEE_FLOAT_EXPONENT>(x.ieee.exponent) << " ";
    std::cout << std::bitset<IEEE_FLOAT_MANTISSA>(x.ieee.mantissa) << std::endl;
}

};
