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
#include <stdio.h>
#include <stdlib.h>

// Block size definitions for quantization
#define BLOCK_SIZE 32
#define Q8_ELEMENTS BLOCK_SIZE

// Macro to clamp a value between a lower and upper bound
#define CLAMP(value, min, max) fmaxf((min), (fminf((value), (max))))

typedef struct Q8 {
    float scalar; /**< Scaling factor for quantization */
    float min; /**< Minimum quantizable value */
    float max; /**< Maximum quantizable value */
    unsigned char quant; /**< Quantized scalar value */
} Q8;

typedef Q8 Q8Row[Q8_ELEMENTS];

// 8-bit integer quantization
Q8 quantize_scalar_q8(float value) {
    Q8 q8;

    q8.max = (1 << 7) - 1; // 127
    q8.min = -(1 << 7); // -128

    // Clamp the input to the specified range
    value = CLAMP(value, q8.min, q8.max);

    // Compute the scalar for dequantization
    q8.scalar = (q8.max - q8.min) / (2 * q8.max);

    // Normalize the value to [-1, 1] and scale
    float normalized = (value - q8.min) / (q8.max - q8.min);
    q8.quant = (unsigned char) roundf((normalized * q8.max));

    return q8;
}

// Function to dequantize a Q8 value back to float
float dequantize_scalar_q8(Q8 q8) {
    // Map back to the original range
    float normalized = (signed char) q8.quant / q8.max;
    return (normalized * (q8.max - q8.min) + q8.min);
}

// 8-bit integer quantization
void quantize_row_q8(const float* input, Q8Row output, int count) {
    assert(input != NULL);
    assert(output != NULL);
    assert(count % Q8_ELEMENTS == 0);

    for (int i = 0; i < count / Q8_ELEMENTS; ++i) {
        for (int j = 0; j < Q8_ELEMENTS; ++j) {
            output[i * Q8_ELEMENTS + j] = quantize_scalar_q8(input[i * Q8_ELEMENTS + j]);
        }
    }
}

void dequantize_row_q8(const Q8Row input, float* output, int count) {
    assert(input != NULL);
    assert(output != NULL);
    assert(count % Q8_ELEMENTS == 0);

    for (int i = 0; i < count / Q8_ELEMENTS; ++i) {
        for (int j = 0; j < Q8_ELEMENTS; ++j) {
            output[i * Q8_ELEMENTS + j] = dequantize_scalar_q8(input[i * Q8_ELEMENTS + j]);
        }
    }
}

void print_floats(const char* label, const float* values, int count) {
    printf("%s: ", label);
    for (int i = 0; i < count; ++i) {
        printf("%.6f ", (double) values[i]);
    }
    printf("\n\n");
}

void print_q8_row(const Q8Row row) {
    printf("Q8 Row: ");
    for (int i = 0; i < Q8_ELEMENTS; ++i) {
        printf("%u ", row[i].quant);
    }
    printf("\n\n");
}

void generate_random_values(float* values, int max_elements, int n) {
    assert(values != NULL);
    assert(max_elements > 0);

    for (int i = 0; i < max_elements; ++i) {
        // Generate a random value in the range [0, 1]
        float normalized = (float) rand() / (float) RAND_MAX;

        // Map the normalized value to the range [-n, n-1]
        values[i] = -n + (normalized * (2 * n - 1));
    }
}

void compute_error(const float* original, const float* dequantized, int count) {
    printf("Quantization Error: ");
    for (int i = 0; i < count; ++i) {
        printf("%.6f ", (double) fabsf(original[i] - dequantized[i]));
    }
    printf("\n");
}

// Stress test for validating out-of-bounds behavior
void run_tests(int seed) {
    // initialize the rng
    srand(seed);
    printf("=== Running tests with seed: %d ===\n\n", seed);

    // Test parameters
    float test_case_1[Q8_ELEMENTS];
    float test_case_2[Q8_ELEMENTS];
    float test_case_3[Q8_ELEMENTS];
    generate_random_values(test_case_1, Q8_ELEMENTS, 127); // [-2^7-1, 2^7-1]
    generate_random_values(test_case_2, Q8_ELEMENTS, 255); // [-2^8-1, 2^8-1]
    generate_random_values(test_case_3, Q8_ELEMENTS, 32767); // [-2^15-1, 2^15-1]

    // @warn Calculating the test count dynamically is unreliable.
    // @warn A variable cannot be used to statically allocate memory to the stack.
    #define TEST_COUNT 3 // @warn Always use a macro for this.
    struct {
        const char* label;
        int count;
        int range;
        float* input;
    } test_cases[TEST_COUNT] = {
        {"Test Case 1", Q8_ELEMENTS, 127, test_case_1},
        {"Test Case 2", Q8_ELEMENTS, 255, test_case_2},
        {"Test Case 3", Q8_ELEMENTS, 32767, test_case_3}
    };

    // Execute test cases
    for (int t = 0; t < TEST_COUNT; ++t) {
        fprintf(stdout, "=== %s (Range: [-%d, %d]) ===\n", test_cases[t].label, test_cases[t].range, test_cases[t].range);
        Q8Row q8_row = {0};
        float dequantized[Q8_ELEMENTS] = {0};

        // Quantize and dequantize
        quantize_row_q8(test_cases[t].input, q8_row, test_cases[t].count);
        dequantize_row_q8(q8_row, dequantized, test_cases[t].count);

        // Print results
        print_floats("Input", test_cases[t].input, test_cases[t].count);
        print_q8_row(q8_row);
        print_floats("Dequantized Output", dequantized, test_cases[t].count);
        compute_error(test_cases[t].input, dequantized, test_cases[t].count);
        printf("\n");
    }

    printf("=== Completed %d test cases ===\n", TEST_COUNT);
}

int main(void) {
    int seed = 1337;
    run_tests(seed);
    return 0;
}
