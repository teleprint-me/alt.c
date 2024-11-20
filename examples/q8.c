/**
 * @file examples/q8.c
 *
 * @brief Prototype for QINT8 quantization.
 *
 * @note
 * - A modern transformer typically consists of 32 blocks.
 * - A block is a single layer composed of 9 sub-layers.
 * - F32 and F16 will typically contain 32 blocks.
 */

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

// Block size definitions for quantization
#define BLOCK_SIZE 32
#define Q8_ELEMENTS BLOCK_SIZE

// Macro to ensure a value falls within a specific range [min, max]
#define MINMAX(value, min, max) fmaxf((min), (fminf((value), (max))))

// Macro to clamp a value between a lower and upper bound
#define CLAMP(value, lower, upper) MINMAX((value), (lower), (upper))

typedef struct {
    float scalar; /**< Scaling factor for quantization */
    float min;
    float max;
    unsigned char quant; /**< Quantized scalar value */
} Q8;

typedef Q8 Q8Row[Q8_ELEMENTS];

// 8-bit integer quantization
Q8 quantize_scalar_q8(float value) {
    Q8 q8;

    // Fixed delta based on the full q8 range
    q8.scalar = 1.0f / 127.0f; // Scaling factor for the full range [-127, 127]
    q8.min = -127.0f;
    q8.max = 127.0f;

    // Clamp the input value to the quantizable range
    float clamped = CLAMP(value, q8.min, q8.max);

    // Quantize by scaling and rounding
    signed char quant = (signed char) roundf(clamped / q8.scalar);

    // Store the quantized value
    q8.quant = (unsigned char) quant + q8.max; // Shift the range [0, 255]

    return q8;
}

float dequantize_scalar_q8(Q8 q8) {
    return q8.scalar * (q8.quant + q8.min); // Shift the range [-127, 127]
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
    printf("\n");
}

void print_q8_row(const Q8Row row) {
    printf("Q8 Row: ");
    for (int i = 0; i < Q8_ELEMENTS; ++i) {
        printf("%u ", row[i].quant);
    }
    printf("\n");
}

int main(void) {
    int seed = 1337;
    srand(seed);

    // Example data
    float data[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE; i++) {
        data[i] = ((float) rand() / (float) RAND_MAX) * 2 - 1; // Generate values in [-1, 1]
    }

    // Print original data
    print_floats("Data", data, BLOCK_SIZE);

    // Quantize and dequantize the data
    Q8Row q8_row;
    float dequantized[BLOCK_SIZE];

    quantize_row_q8(data, q8_row, BLOCK_SIZE);
    dequantize_row_q8(q8_row, dequantized, BLOCK_SIZE);

    // Print quantized and dequantized results
    print_q8_row(q8_row);
    print_floats("Dequantized", dequantized, BLOCK_SIZE);

    return 0;
}
