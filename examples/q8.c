/**
 * @file examples/q8.c
 * 
 * @brief Prototype for QINT8 quantization.
 */

#include <stdlib.h>
#include <stdio.h>

#define BLOCK_SIZE 32
#define Q8_ELEMENTS BLOCK_SIZE

typedef struct {
    float scalar;  /**< Scaling factor for quantization */
    float min;
    float max;
    unsigned char quant; /**< Quantized scalar value */
} Q8;

typedef Q8 Q8Row[Q8_ELEMENTS];

// 8-bit integer quantization (unsigned representation)
Q8 quantize_scalar_q8(float value) {
    Q8 q8;

    // Set the Q8 range
    q8.min = -128.0f;
    q8.max = 127.0f;

    // Fixed-scalar for the full Q8 range
    q8.scalar = (q8.max - q8.min) / 255.0f; // Scaling factor for [-128, 127]

    // Clamp the input to ensure it lies within the valid range
    float clamped = CLAMP(value, q8.min, q8.max);

    // Quantize by scaling, mapping to [0, 255], and rounding
    float normalized = (clamped - q8.min) / (q8.max - q8.min); // Normalize to [0, 1]
    q8.quant = (unsigned char) roundf(normalized * 255.0f); // Scale to [0, 255]

    return q8;
}

float dequantize_scalar_q8(Q8 q8) {
    // Reverse the unsigned mapping
    float normalized = q8.quant / 255.0f; // Normalize to [0, 1]
    return normalized * (q8.max - q8.min) + q8.min; // Map back to [-128, 127]
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
        data[i] = ((float) rand() / (float) RAND_MAX) * 2 - 1;
    }

    return 0;
}
