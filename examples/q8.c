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

// Macro to clamp a value between a lower and upper bound
#define CLAMP(value, min, max) fmaxf((min), (fminf((value), (max))))

typedef union {
    float value; /**< Floating-point value */
    unsigned int bits; /**< Raw bit representation */
} FloatBits;

typedef struct Q8 {
    float scalar;    /**< Scaling factor for quantization */
    float min;       /**< Minimum quantizable value */
    float max;       /**< Maximum quantizable value */
    unsigned char quant; /**< Quantized scalar value */
} Q8;

typedef Q8 Q8Row[Q8_ELEMENTS];

// Floating-point encoding and decoding
unsigned int encode_float32_to_bits(float value) {
    FloatBits raw;
    raw.value = value;
    return raw.bits;
}

float decode_bits_to_float32(unsigned int bits) {
    FloatBits raw;
    raw.bits = bits;
    return raw.value;
}

// 8-bit integer quantization
Q8 quantize_scalar_q8(float value) {
    Q8 q8;

    q8.min = -127.0f;
    q8.max = 127.0f;

    // Compute the scaling factor (scalar)
    float abs_value = fabsf(value); // get the absolute value
    q8.scalar = abs_value / fmaxf(q8.max, abs_value); // bind the value to [0, 127]

    // Clamp the input value to the quantizable range
    float clamped = CLAMP(value, q8.min, q8.max);

    // Compute the quantized value
    signed char quant = (signed char) roundf(clamped / q8.scalar);

    // Shift to unsigned range
    q8.quant = (unsigned char) (quant + 127);

    return q8;
}

// Reverse the scaling to reconstruct the original value
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

void test_q8_prototype(void) {
    float input[Q8_ELEMENTS] = {-127.0f, -64.0f, -32.0f, -1.0f, 0.0f, 1.0f, 32.0f, 64.0f, 127.0f};
    Q8Row q8_row;
    float output[Q8_ELEMENTS];

    // Quantize and dequantize the row
    quantize_row_q8(input, q8_row, Q8_ELEMENTS);
    dequantize_row_q8(q8_row, output, Q8_ELEMENTS);

    // Print results
    print_floats("Input", input, Q8_ELEMENTS);
    print_q8_row(q8_row);
    print_floats("Dequantized Output", output, Q8_ELEMENTS);
}

void test_q8_concept(void) {
    int seed = 1337;
    srand(seed);

    // Example data
    float data[BLOCK_SIZE];
    for (int i = 0; i < BLOCK_SIZE; i++) {
        data[i] = ((float) rand() / (float) RAND_MAX) * 1000.0f - 500.0f; // Generate values in [-500, 500]
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
}

int main(void) {
    test_q8_prototype();
    return 0;
}
