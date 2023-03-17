#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <gmp.h>
#include <mpfr.h>

#define TEST_SIZE 10000
#define TEST_STEP_SIZE 100000
#define IMPRECISE_PRECISION {11, 24, 53}
#define PRECISE_PRECISION 1024
#define DEBUG false

void randomize(mpfr_t *random_number, gmp_randstate_t random_state, uint exponent_length) {
  // Set mantissa
  mpfr_urandomb(*random_number, random_state);

  // Set sign
  int sign = rand() % 2;
  if (sign) {
    mpfr_mul_si(*random_number, *random_number, -1, MPFR_RNDN);
  }

  // Set exponent
  int exponent = (rand() % (int)pow(2, exponent_length)) - (int)pow(2, exponent_length-1);
  mpfr_mul_2si (*random_number, *random_number, exponent, MPFR_RNDN);
}

int main (void) {
  int precisions[] = IMPRECISE_PRECISION;
  size_t size_of_precision = sizeof(precisions) / sizeof(int);
  for (size_t index = 0; index < size_of_precision; index++) {
    // Set the random state for reproducibility
    gmp_randstate_t random_state;
    gmp_randinit_default(random_state);
    gmp_randseed_ui(random_state, 42);

    // Initialize relevant variables
    int precision = precisions[index];
    mpfr_t original_sum, HEAR_sum, true_sum, random_number, encrypted_error, original_error;
    mpfr_t noises[TEST_STEP_SIZE];
    for (size_t step = 0; step < TEST_STEP_SIZE; step++) {
      mpfr_init2(noises[step], precision);
      mpfr_set_ui(noises[step], 0.0, MPFR_RNDN);
      while (!mpfr_cmp_d(noises[step], 0.0))
        mpfr_urandomb(noises[step], random_state);
    }
    mpfr_inits2(PRECISE_PRECISION, true_sum, encrypted_error, original_error, NULL);
    mpfr_inits2(precision, original_sum, HEAR_sum, random_number, NULL);

    // Prepare file for saving results
    char *filename = (char *)malloc(sizeof(char)*100);
    sprintf(filename, "./results/%d_float_addition.csv", precision);
    FILE *results;
    results=fopen(filename, "w");

    for (size_t i = 0; i < TEST_SIZE; i++) {
      mpfr_set_ui(true_sum, 0.0, MPFR_RNDN);
      mpfr_set_ui(original_sum, 0.0, MPFR_RNDN);
      mpfr_set_ui(HEAR_sum, 0.0, MPFR_RNDN);
      for (size_t j = 0; j < TEST_STEP_SIZE; j++) {
        randomize(&random_number, random_state, 11);
        mpfr_add(true_sum, true_sum, random_number, MPFR_RNDN);
        mpfr_add(original_sum, original_sum, random_number, MPFR_RNDN);
        mpfr_mul(random_number, random_number, noises[0], MPFR_RNDN);
        mpfr_add(HEAR_sum, HEAR_sum, random_number, MPFR_RNDN);
      }
      mpfr_div(HEAR_sum, HEAR_sum, noises[0], MPFR_RNDN);
      mpfr_sub(encrypted_error, HEAR_sum, true_sum, MPFR_RNDN);
      mpfr_div(encrypted_error, encrypted_error, true_sum, MPFR_RNDN);
      mpfr_abs(encrypted_error, encrypted_error, MPFR_RNDN);
      double encrypted_error_d = log2(mpfr_get_d(encrypted_error, MPFR_RNDN));
      mpfr_sub(original_error, original_sum, true_sum, MPFR_RNDN);
      mpfr_div(original_error, original_error, true_sum, MPFR_RNDN);
      mpfr_abs(original_error, original_error, MPFR_RNDN);
      double original_error_d = log2(mpfr_get_d(original_error, MPFR_RNDN));
      // mpfr_sub(encrypted_error, encrypted_error, original_error, MPFR_RNDN);
      // mpfr_div(encrypted_error, encrypted_error, true_sum, MPFR_RNDN);
      double total_error = mpfr_get_d(encrypted_error, MPFR_RNDN);
      if (DEBUG)
        printf("Sum is %10f %10f %10e ", encrypted_error_d, original_error_d, total_error);
      
      // Save the results
      mpfr_out_str(results, 10, 0, encrypted_error, MPFR_RNDN);
      fprintf(results, ", encrypted\n");
      mpfr_out_str(results, 10, 0, original_error, MPFR_RNDN);
      fprintf(results, ", original\n");
      if (mpfr_nan_p(encrypted_error)) {
        printf("Sum is %10f %10f %10e ", encrypted_error_d, original_error_d, total_error);
      }
    }

    // Clean the resources
    fclose(results);
    free(filename);
    printf("Finished precision %d\n", precision);
    fflush(stdout);
  }
  return 0;
}