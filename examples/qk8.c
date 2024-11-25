/**
 * @file examples/qk8.c
 */

#include <assert.h>
#include <math.h> // For round()
#include <stdint.h> // For aliased data types, e.g. uint8_t
#include <stdio.h>
#include <stdlib.h> // For rand() and srand()
#include <time.h> // For seeding random number generator

#define MAX_SAMPLES 10

typedef union {
    float value; /**< Floating-point value */
    uint32_t bits; /**< Raw bit representation */
} Float32;

// Encode a float into an 8-bit floating-point representation
uint8_t encode_float8(float value) {
    if (value == 0.0f) {
        return 0; // Encoded as all zeros
    }

    Float32 encoder = {.value = value};

    // Extract IEEE-754 components
    uint32_t sign = (encoder.bits >> 31) & 0x1;
    uint32_t exponent = (encoder.bits >> 23) & 0xff;
    uint32_t mantissa = encoder.bits & 0x7fffff;

    // Define bias parameters
    uint32_t e_bias_32 = 127;
    uint32_t e_bias_8 = 3;

    // Define exponent limits
    uint32_t e_max = 7;
    uint32_t e_min = 0;

    // Calculate compressed exponent
    int8_t e_compressed = fmaxf(fminf(exponent - e_bias_32 + e_bias_8, e_max), e_min);

    // Calculate compressed mantissa (top 4 bits of the 23-bit mantissa)
    uint8_t m_compressed = (mantissa >> 19) & 0xf;

    // Pack into an 8-bit integer
    return (uint8_t) ((sign << 7) | (e_compressed << 4) | m_compressed);
}

// Decode an 8-bit floating-point representation back to a float
float decode_float8(uint8_t bits) {
    // Extract fields
    uint8_t sign = (bits >> 7) & 0x01;
    uint8_t exponent = (bits >> 4) & 0x07;
    uint8_t mantissa = bits & 0x0F;

    // Define parameters
    uint32_t e_bias_32 = 127;
    uint32_t e_bias_8 = 3;

    // Expand exponent
    int32_t e_expanded = exponent - e_bias_8 + e_bias_32;

    // Expand mantissa with implicit leading 1
    float m_expanded = 1.0f + (mantissa / 16.0f);

    // Reconstruct float
    float result = ldexpf(m_expanded, e_expanded - e_bias_32);
    return sign ? -result : result;
}

// Function to sample floating-point values in the range [-n, n-1]
void sampler(double* x, int max_elements, int n) {
    assert(x != NULL);
    assert(max_elements > 0);
    assert(n > 1);

    for (int i = 0; i < max_elements; ++i) {
        double normalized = (double) rand() / (double) RAND_MAX;
        x[i] = -n + (normalized * (2 * n - 1));
    }
}

// Compute absolute and relative errors
void error(double original, double reconstructed, double* abs_error, double* rel_error) {
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

    printf("Randomly Generated Samples:\n");
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
