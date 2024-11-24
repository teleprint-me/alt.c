/**
 * @file examples/qk8.c
 */

#include <assert.h>
#include <math.h>   // For round()
#include <stdio.h>
#include <stdlib.h> // For rand() and srand()
#include <time.h>   // For seeding random number generator
#include <stdint.h> // For aliased data types, e.g. uint8_t

#define MAX_SAMPLES 10
#define BIAS 3

typedef struct {
    uint8_t sign;
    uint8_t exponent;
    uint8_t mantissa;
} Float8;

// Encode a float into an 8-bit floating-point representation
uint8_t encode_float8(float value) {
    Float8 f8;
    uint8_t result = 0;

    // Extract sign
    f8.sign = value < 0 ? 1 : 0;
    if (f8.sign) value = -value;

    // Normalize the value to extract exponent and mantissa
    int exponent;
    float mantissa = frexp(value, &exponent); // Decomposes value = mantissa * 2^exponent

    // Adjust exponent with bias
    exponent += BIAS;

    // Ensure the exponent is within range
    if (exponent < 0) exponent = 0;
    if (exponent > 7) exponent = 7;

    // Scale and truncate mantissa to 4 bits
    f8.exponent = exponent & 0x07; // 3 bits
    f8.mantissa = (uint8_t)(mantissa * 16) & 0x0F; // 4 bits

    // Pack into an 8-bit value
    result = (f8.sign << 7) | (f8.exponent << 4) | f8.mantissa;

    return result;
}

// Decode an 8-bit floating-point representation back to a float
float decode_float8(uint8_t encoded) {
    Float8 f8;

    // Extract fields
    f8.sign = (encoded >> 7) & 0x01;
    f8.exponent = (encoded >> 4) & 0x07;
    f8.mantissa = encoded & 0x0F;

    // Compute exponent and mantissa
    int true_exponent = f8.exponent - BIAS;
    float mantissa = 1.0 + (f8.mantissa / 16.0); // Implicit leading 1

    // Reconstruct the value
    float value = ldexp(mantissa, true_exponent); // Combines mantissa * 2^exponent
    return f8.sign ? -value : value;
}

// Function to sample floating-point values in the range [-n, n-1]
void sampler(double *x, int max_elements, int n) {
    assert(x != NULL);
    assert(max_elements > 0);
    assert(n > 1);

    for (int i = 0; i < max_elements; ++i) {
        double normalized = (double) rand() / (double) RAND_MAX;
        x[i] = -n + (normalized * (2 * n - 1));
    }
}

// Compute absolute and relative errors
void error(double original, double reconstructed, double *abs_error, double *rel_error) {
    *abs_error = fabs(original - reconstructed);
    *rel_error = (fabs(original) > 1e-6) ? (*abs_error / fabs(original)) : 0.0;
}

int main() {
    srand(1); // Fixed seed for reproducibility

    // Generate sampled data
    double sampled[MAX_SAMPLES];
    sampler(sampled, MAX_SAMPLES, 2);

    // Initialize error accumulators
    double total_abs_error = 0.0, total_rel_error = 0.0;

    printf("\nRandomly Generated Samples:\n");
    for (int i = 0; i < MAX_SAMPLES; i++) {
        double x = sampled[i];
        printf("\nInput: %.6f\n", x);

        uint8_t q = encode_float8(x); // Quantize
        printf("Quantized: 0x%02X\n", q);

        double x_prime = decode_float8(q); // Dequantize
        printf("Dequantized: %.6f\n", x_prime);

        // Calculate errors
        double abs_error, rel_error;
        error(x, x_prime, &abs_error, &rel_error);
        total_abs_error += abs_error;
        total_rel_error += rel_error;

        printf("Absolute Error: %.6f, Relative Error: %.2f%%\n", abs_error, rel_error * 100);
    }

    printf("\nAverage Absolute Error: %.6f\n", total_abs_error / MAX_SAMPLES);
    printf("Average Relative Error: %.2f%%\n", (total_rel_error / MAX_SAMPLES) * 100);

    return 0;
}
