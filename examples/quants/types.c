/**
 * @file examples/data_types.c
 * @brief Example usage of the data types API.
 *
 * Demonstrates:
 * - Scalar quantization and dequantization.
 * - Row-based quantization and dequantization.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "interface/data_types.h"

// override block size
#undef BLOCK_SIZE
// define number of samples
#define BLOCK_SIZE 10

// define sample rate
#define STEP_SIZE 1

void print_floats(const char* label, const float* values, int count) {
    printf("%s: ", label);
    for (int i = 0; i < count; ++i) {
        printf("%.6f ", (double) values[i]);
    }
    printf("\n");
}

void print_f16_row(const unsigned short* values, int count) {
    printf("F16 Row: ");
    for (int i = 0; i < count; ++i) {
        printf("%04x ", values[i]);
    }
    printf("\n");
}

void print_q8_row(const Q8Row row) {
    printf("Q8 Row: ");
    for (int i = 0; i < BLOCK_SIZE; ++i) {
        printf("%02x ", row[i].bits);
    }
    printf("\n");
}

void print_q4_row(const Q4Row row) {
    printf("Q4 Row: ");
    for (int i = 0; i < BLOCK_SIZE / 2; ++i) {
        printf("%02x ", row[i].bits);
    }
    printf("\n");
}

// Function to sample floating-point values in the range [-n, n-1]
void sampler(float* x, int length, int range) {
    assert(x != NULL);
    assert(length > 0);
    assert(range > 1);

    for (int i = 0; i < length; ++i) {
        float normalized = (float) rand() / (float) RAND_MAX;
        x[i] = -(range + 1) + (normalized * ((2 * range) - 1));
    }
}

int main(void) {
    srand(1337); // fixed seed for reproducible results

    // Sampled dataset
    float data[BLOCK_SIZE];
    // Set sampled interval to [-10, 9]
    sampler(data, BLOCK_SIZE, 10);

    // Print original data
    print_floats("Original Data", data, BLOCK_SIZE);

    // Scalar Quantization Examples

    // f16
    printf("\n-- Scalar F16 Example --\n");
    unsigned short f16 = quantize_scalar_fp16(data[0]);
    float f16_dequantized = dequantize_scalar_fp16(f16);
    printf("F16 input (x): %.6f\n", (double) data[0]);
    printf("F16 quantized (y): %04x\n", f16);
    printf("F16 dequantized (x'): %.6f\n", (double) f16_dequantized);

    // q8
    printf("\n-- Scalar Q8 Example --\n");
    Q8 q8 = quantize_scalar_q8(data[1]);
    float q8_dequantized = dequantize_scalar_q8(q8);
    printf("Q8 input (x): %.6f\n", (double) data[1]);
    printf("Q8 quantized (y): %02x\n", q8.bits);
    printf("Q8 dequantized (x'): %.6f\n", (double) q8_dequantized);

    // q4
    printf("\n-- Scalar Q4 Example --\n");
    Q4 q4 = quantize_scalar_q4((double) data[2], (double) data[3]);
    float q4_dequantized_0 = dequantize_scalar_q4_index(q4, 0);
    float q4_dequantized_1 = dequantize_scalar_q4_index(q4, 1);
    printf("Q4 input (x_1, x_2): %.6f, %.6f\n", (double) data[2], (double) data[3]);
    printf("Q4 quantized (y): %02x\n", q4.bits);
    printf("Q4 dequantized (x_1', x_2'): %.6f, %.6f\n", (double) q4_dequantized_0, (double) q4_dequantized_1);

    // Row Quantization Examples

    // f16
    printf("\n-- Row F16 Example --\n");
    unsigned short f16_row[BLOCK_SIZE];
    quantize_row_fp16(data, f16_row, BLOCK_SIZE, STEP_SIZE);
    print_f16_row(f16_row, BLOCK_SIZE);
    float f16_row_dequantized[BLOCK_SIZE];
    dequantize_row_fp16(f16_row, f16_row_dequantized, BLOCK_SIZE, STEP_SIZE);
    print_floats("F16 Dequantized Row", f16_row_dequantized, BLOCK_SIZE);

    // q8
    printf("\n-- Row Q8 Example --\n");
    Q8Row q8_row;
    quantize_row_q8(data, q8_row, BLOCK_SIZE, STEP_SIZE);
    print_q8_row(q8_row);
    float q8_row_dequantized[BLOCK_SIZE];
    dequantize_row_q8(q8_row, q8_row_dequantized, BLOCK_SIZE, STEP_SIZE);
    print_floats("Q8 Dequantized Row", q8_row_dequantized, BLOCK_SIZE);

    // q4
    printf("\n-- Row Q4 Example --\n");
    Q4Row q4_row;
    quantize_row_q4(data, q4_row, BLOCK_SIZE, STEP_SIZE);
    print_q4_row(q4_row);
    float q4_row_dequantized[BLOCK_SIZE];
    dequantize_row_q4(q4_row, q4_row_dequantized, BLOCK_SIZE, STEP_SIZE);
    print_floats("Q4 Dequantized Row", q4_row_dequantized, BLOCK_SIZE);

    return 0;
}
