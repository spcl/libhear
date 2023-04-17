#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <gmp.h>
#include <mpfr.h>
#include <unistd.h>
#include <string.h>

#define IMPRECISE_PRECISION {11, 24, 53}
#define PRECISE_PRECISION 1024
#define DEBUG false
#define ROUNDING MPFR_RNDN

void true_random(mpfr_t *random_number, gmp_randstate_t random_state, int exponent_length) {
  // Set mantissa
  mpfr_urandomb(*random_number, random_state);

  // Set sign
  int sign = rand() % 2;
  if (sign) {
    mpfr_mul_si(*random_number, *random_number, -1, ROUNDING);
  }

  // Set exponent
  if (exponent_length > 0) {
    int exponent = (rand() % (int)pow(2, exponent_length)) - (int)pow(2, exponent_length-1);
    mpfr_mul_2si (*random_number, *random_number, exponent, ROUNDING);
  }
}

void uniform_random(mpfr_t *random_number, gmp_randstate_t random_state, double min, double max) {
  // Set mantissa
  mpfr_urandom(*random_number, random_state, ROUNDING);

  // Scale to the minimum and maximum
  mpfr_mul_d(*random_number, *random_number, max-min, ROUNDING);
  mpfr_add_d(*random_number, *random_number, min, ROUNDING);
}

void gaussian_random(mpfr_t *random_number, gmp_randstate_t random_state, double sigma, double mu) {
  // Set mantissa
  mpfr_nrandom(*random_number, random_state, ROUNDING);

  // Set the mean and exponent
  mpfr_mul_d(*random_number, *random_number, sigma, ROUNDING);
  mpfr_add_d(*random_number, *random_number, mu, ROUNDING);
}

int main (int argc, char *argv[]) {
  // Parse the arguments
  int opt;
  enum {RANDOM, UNIFORM, GAUSSIAN} mode = RANDOM;
  int test_size = 10000;
  int test_step_size = 100000;
  double low = -1e5;
  double high = 1e5;
  double mu = 0;
  double sigma = 1e5;
  char name[100] = {'\0'};
  while ((opt = getopt(argc, argv, "gus:t:l:h:m:o:n:")) != -1) {
    switch (opt) {
    case 'g': mode = GAUSSIAN; break;
    case 'u': mode = UNIFORM; break;
    case 's': test_size = atoi(optarg); break;
    case 't': test_step_size = atoi(optarg); break;
    case 'l': low = atof(optarg); break;
    case 'h': high = atof(optarg); break;
    case 'm': mu = atof(optarg); break;
    case 'o': sigma = atof(optarg); break;
    case 'n': strcpy(name, optarg); break;
    default:
        fprintf(stderr, "Usage: %s [-gustlhmon] [file...]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  // Set the variables
  int precisions[] = IMPRECISE_PRECISION;
  size_t size_of_precision = sizeof(precisions) / sizeof(int);
  for (size_t index = 0; index < size_of_precision; index++) {
    // Set the random state for reproducibility
    gmp_randstate_t random_state;
    gmp_randinit_default(random_state);
    gmp_randseed_ui(random_state, 42);

    // Initialize relevant variables
    int precision = precisions[index];
    mpfr_t original_sum, HEAR_sum0, HEAR_sum1, HEAR_sum2, true_sum, random_number, encrypted_error0, encrypted_error1, encrypted_error2, original_error, noise, random_encrypted0, random_encrypted1, random_encrypted2;
    mpfr_init2(noise, precision);
    mpfr_set_ui(noise, 0.0, ROUNDING);
    while (!mpfr_cmp_d(noise, 0.0))
      true_random(&noise, random_state, 10);
    mpfr_inits2(PRECISE_PRECISION, true_sum, encrypted_error0, encrypted_error1, encrypted_error2, original_error, NULL);
    mpfr_inits2(precision, original_sum, random_number, NULL);
    mpfr_inits2(precision, random_encrypted0, HEAR_sum0, NULL);
    mpfr_inits2(precision-1, random_encrypted1, HEAR_sum1, NULL);
    mpfr_inits2(precision-2, random_encrypted2, HEAR_sum2, NULL);

    // Prepare file for saving results
    char filename[200] = {'\0'};
    sprintf(filename, "./tests/accuracy/results/%d_float_addition_%s.csv", precision, name);
    FILE *results;
    results = fopen(filename, "w");
    fprintf(results, "error,type\n");

    for (size_t i = 0; i < test_size; i++) {
      // Reset variables
      mpfr_set_ui(true_sum, 0.0, ROUNDING);
      mpfr_set_ui(original_sum, 0.0, ROUNDING);
      mpfr_set_ui(HEAR_sum0, 0.0, ROUNDING);
      mpfr_set_ui(HEAR_sum1, 0.0, ROUNDING);
      mpfr_set_ui(HEAR_sum2, 0.0, ROUNDING);

      // Conduct the summation
      for (size_t j = 0; j < test_step_size; j++) {
        if (mode == RANDOM)
          true_random(&random_number, random_state, 10);
        else if (mode == UNIFORM)
          uniform_random(&random_number, random_state, low, high);
        else if (mode == GAUSSIAN)
          gaussian_random(&random_number, random_state, sigma, mu);
        mpfr_add(true_sum, true_sum, random_number, ROUNDING);
        mpfr_add(original_sum, original_sum, random_number, ROUNDING);
        mpfr_mul(random_encrypted0, random_number, noise, ROUNDING);
        mpfr_mul(random_encrypted1, random_number, noise, ROUNDING);
        mpfr_mul(random_encrypted2, random_number, noise, ROUNDING);
        mpfr_add(HEAR_sum0, HEAR_sum0, random_encrypted0, ROUNDING);
        mpfr_add(HEAR_sum1, HEAR_sum1, random_encrypted1, ROUNDING);
        mpfr_add(HEAR_sum2, HEAR_sum2, random_encrypted2, ROUNDING);
      }

      // Decrypt and compare the results
      mpfr_div(HEAR_sum0, HEAR_sum0, noise, ROUNDING);
      mpfr_sub(encrypted_error0, HEAR_sum0, true_sum, ROUNDING);
      mpfr_div(encrypted_error0, encrypted_error0, true_sum, ROUNDING);
      mpfr_abs(encrypted_error0, encrypted_error0, ROUNDING);
      mpfr_div(HEAR_sum1, HEAR_sum1, noise, ROUNDING);
      mpfr_sub(encrypted_error1, HEAR_sum1, true_sum, ROUNDING);
      mpfr_div(encrypted_error1, encrypted_error1, true_sum, ROUNDING);
      mpfr_abs(encrypted_error1, encrypted_error1, ROUNDING);
      mpfr_div(HEAR_sum2, HEAR_sum2, noise, ROUNDING);
      mpfr_sub(encrypted_error2, HEAR_sum2, true_sum, ROUNDING);
      mpfr_div(encrypted_error2, encrypted_error2, true_sum, ROUNDING);
      mpfr_abs(encrypted_error2, encrypted_error2, ROUNDING);
      mpfr_sub(original_error, original_sum, true_sum, ROUNDING);
      mpfr_div(original_error, original_error, true_sum, ROUNDING);
      mpfr_abs(original_error, original_error, ROUNDING);

      // Save the results if the original sum is not infinite
      if (mpfr_number_p(original_error)) {
        mpfr_out_str(results, 10, 0, encrypted_error0, ROUNDING);
        fprintf(results, ",HEAR0\n");
        mpfr_out_str(results, 10, 0, encrypted_error1, ROUNDING);
        fprintf(results, ",HEAR1\n");
        mpfr_out_str(results, 10, 0, encrypted_error2, ROUNDING);
        fprintf(results, ",HEAR2\n");
        mpfr_out_str(results, 10, 0, original_error, ROUNDING);
        fprintf(results, ",native\n");
      }
      if (DEBUG) {
        double encrypted_error_d = log2(mpfr_get_d(encrypted_error0, ROUNDING));
        double original_error_d = log2(mpfr_get_d(original_error, ROUNDING));
        double total_error = mpfr_get_d(encrypted_error0, ROUNDING);
        printf("%10f %10f %10e ", encrypted_error_d, original_error_d, total_error);
        if (mpfr_nan_p(encrypted_error0)) {
          printf("Encountered a NaN");
        }
      }
    }

    // Clean the resources
    fclose(results);
    printf("Finished precision %d\n", precision);
    fflush(stdout);
  }
  return 0;
}