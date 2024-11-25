/**
 * @file examples/q8.c
 *
 * @brief Prototype for QINT8 quantization.
 *
 * @note A modern transformer typically consists of 32 blocks.
 *   - A block is a single layer composed of 9 sub-layers.
 *   - Each sub-layer usually consists of one of two shapes.
 *     - @note Shapes represent row major order.
 *     - Shape 1: Vector: (n, 1) -> n (cols) x 1 (rows)
 *     - Shape 2: Matrix: (n, m) -> n (cols) x m (rows)
 * - @note F32, F16, I16, and I8 will typically contain 32 blocks.
 */

#include <assert.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_SAMPLES 10

// Quantization structure
typedef struct Quant {
    float scalar; /**< Scaling factor for quantization of input */
    uint8_t bits; /**< Quantized value */
} Quant;

typedef Quant Q8;

// 8-bit integer quantization
Q8 quantize_q8(float value) {
    Q8 q8;

    // Set the range for the integer domain [-128, 127]
    int z_domain = 255;

    // Calculate the dynamic range for the real domain [-value, value]
    float r_domain = fabsf(value); // reflect and compound input

    // if value is greater than z_max or value is less than z_min, otherwise 1
    float squeeze = 1;
    if (r_domain > z_domain) {
        squeeze = z_domain / r_domain; // ratio of the effective range
    }

    // Calculate the scaling factor and preserve the sign
    q8.scalar = (squeeze * r_domain) / z_domain;
    q8.scalar = (value >= 0) ? q8.scalar : -q8.scalar;

    // Quantize the value
    q8.bits = (uint8_t) roundf(value / q8.scalar);

    return q8;
}

// Reconstruct the real value using the scalar
float dequantize_q8(Q8 q8) {
    return (float) (q8.scalar * q8.bits); // / q8.alpha;
}

// Function to sample floating-point values in the range [-n, n-1]
void sampler(double* x, int length, int range) {
    assert(x != NULL);
    assert(length > 0);
    assert(range > 1);

    for (int i = 0; i < length; ++i) {
        double normalized = (double) rand() / (double) RAND_MAX;
        x[i] = -(range + 1) + (normalized * ((2 * range) - 1));
    }
}

// Compute absolute and relative errors
void error(double x, double x_prime, double* abs_error, double* rel_error) {
    *abs_error = fabs(x - x_prime);
    *rel_error = (fabs(x) > 1e-6) ? (*abs_error / fabs(x)) : 0.0;
}

// Stress test for validating out-of-bounds behavior
#define TEST_COUNT 3

typedef struct TestCase {
    const char* label;
    int exponent;
} TestCase;

void run_tests(int seed) {
    // initialize the rng
    srand(seed);
    printf("=== Running tests with seed: %d ===\n\n", seed);

    // Test parameters
    double input[MAX_SAMPLES];

    TestCase test_cases[TEST_COUNT] = {
        {"8-bit Signed Test",  127 },
        {"8-bit Unsigned Test", 255},
        {"16-bit Signed Test", 32535}
    };

    // Initialize error accumulators
    double total_abs_error = 0.0, total_rel_error = 0.0;
    for (int t = 0; t < TEST_COUNT; t++) {
        fprintf(
            stdout,
            "=== %s (Range: [%d, %d]) ===\n",
            test_cases[t].label,
            -(test_cases[t].exponent + 1),
            test_cases[t].exponent
        );

        // Update the sampled inputs
        sampler(input, MAX_SAMPLES, test_cases[t].exponent);
        for (int sample = 0; sample < MAX_SAMPLES; sample++) {
            // Get sample, calculate quant, and dequant
            double x = input[sample];
            Q8 q = quantize_q8(x);
            double x_prime = dequantize_q8(q);
            printf("Input: %.6f, Quant: %u, Prime: %.6f\n", x, q.bits, x_prime);

            // Calculate accumulated errors
            double abs_error, rel_error;
            error(x, x_prime, &abs_error, &rel_error);
            total_abs_error += abs_error;
            total_rel_error += rel_error;
            printf("Absolute Error: %.6f, Relative Error: %.2f%%\n\n", abs_error, rel_error * 100);
        }
    }

    printf("Average Absolute Error: %.6f\n", total_abs_error / MAX_SAMPLES);
    printf("Average Relative Error: %.2f%%\n", (total_rel_error / MAX_SAMPLES) * 100);
    printf("=== Completed %d test cases ===\n", TEST_COUNT);
}

int main(void) {
    run_tests(1337);
    return 0;
}
