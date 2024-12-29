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

typedef struct QuantMetaData {
    uint8_t bits;
    float alpha;
    float step_size;
    float residual;
} QuantMetaData;

// Quantization structure
typedef struct Quant {
    uint8_t bits; /**< Quantized value with baked residual */
    float scalar; /**< Scaling factor for quantization */
} QuantBits;

typedef QuantBits Q8;

QuantMetaData quantize_meta_data(float value, float r_domain, int32_t z_domain) {
    QuantMetaData m;

    // Calculate squeezing ratio
    m.alpha = (r_domain > z_domain) ? z_domain / r_domain : 1.0f;
    // Calculate the base step size
    m.step_size = r_domain / z_domain; // Decoupled from scalar
    // Quantize the value using the base step size (exponent/mantissa?)
    m.bits = roundf(value / m.step_size);
    // Calculate the residual precision (bias?)
    m.residual = (value - (m.bits * m.step_size));

    return m;
}

float quantize_scalar_input(QuantMetaData m) {
    return m.step_size * m.alpha + m.residual;
}

// 8-bit quantization with residual baking
Q8 quantize_q8(float value) {
    Q8 q;
    // Define integer domain
    int z_domain = 255;
    // Reflect and compute effective real domain
    float r_domain = fabsf(value);

    // Special case for zero
    if (r_domain == 0.0f) {
        q.scalar = 1.0f;
        q.bits = 0;
        return q;
    }

    // Extract the components from the input, real, and integer domains
    QuantMetaData m = quantize_meta_data(value, r_domain, z_domain);
    // Calculate the scalar based on the squeezed range
    q.scalar = quantize_scalar_input(m);
    // Quantize the value
    q.bits = (uint8_t) (roundf(m.bits));

    return q;
}

// Dequantize the value
float dequantize_q8(Q8 q) {
    return (float) (q.bits * q.scalar);
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
        {"8-bit Signed Test",   127  },
        {"8-bit Unsigned Test", 255  },
        {"16-bit Signed Test",  32535}
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
