#include "encrypt.hpp"

const encr_key_t max_encr_key = 42; // std::numeric_limits<encr_key_t>::max();
std::random_device rd {};
std::mt19937 encr_key_generator(rd());
std::mt19937 encr_noise_generator;
std::uniform_int_distribution<encr_key_t> encr_key_distr(0, max_encr_key);

encr_key_t generate_encr_key()
{
    return encr_key_distr(encr_key_generator);
}

encr_key_t prng(encr_key_t seed)
{
    encr_noise_generator.seed(seed);
    return encr_key_distr(encr_noise_generator);
    /*
     * Warning:
     * signed integer overflow is UB.
     * TODO: workaround with casting int to uint and do all the stuff safel.
     *
     //return encr_noise_generator();
     *
     */
}
