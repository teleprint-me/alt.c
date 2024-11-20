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

#include "data_types.h"

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
        printf("%u ", row[i].scalar);
    }
    printf("\n");
}

void print_q4_row(const Q4Row row) {
    printf("Q4 Row: ");
    for (int i = 0; i < Q4_NIBBLES; ++i) {
        printf("%02x ", row[i].scalar);
    }
    printf("\n");
}

int main(void) {
    int seed = 1337;
    srand(seed);

    // Example data
    float data[DATA_BLOCK_SIZE];
    for (int i = 0; i < DATA_BLOCK_SIZE; i++) {
        data[i] = (float) rand() / (float) RAND_MAX;
    }

    // Print original data
    print_floats("Original Data", data, DATA_BLOCK_SIZE);

    // Scalar Quantization Examples

    // f16
    printf("\n-- Scalar f16 Example --\n");
    unsigned short f16 = quantize_scalar_fp16(data[0]);
    float f16_dequantized = dequantize_scalar_fp16(f16);
    printf("f16 Quantized: %04x\n", f16);
    printf("f16 Dequantized: %.6f\n", (double) f16_dequantized);

    // q8
    printf("\n-- Scalar q8 Example --\n");
    Q8 q8 = quantize_scalar_q8(data[1]);
    float q8_dequantized = dequantize_scalar_q8(q8);
    printf("q8 Quantized: %u\n", q8.scalar);
    printf("q8 Dequantized: %.6f\n", (double) q8_dequantized);

    // q4
    printf("\n-- Scalar q4 Example --\n");
    Q4 q4 = quantize_scalar_q4(data[2], data[3]);
    float q4_dequantized_0 = dequantize_scalar_q4(q4, 0);
    float q4_dequantized_1 = dequantize_scalar_q4(q4, 1);
    printf("q4 Quantized: %02x\n", q4.scalar);
    printf("q4 Dequantized (0): %.6f\n", (double) q4_dequantized_0);
    printf("q4 Dequantized (1): %.6f\n", (double) q4_dequantized_1);

    // Row Quantization Examples

    // f16
    printf("\n-- Row f16 Example --\n");
    unsigned short f16_row[DATA_BLOCK_SIZE];
    quantize_row_fp16(data, f16_row, DATA_BLOCK_SIZE);
    float f16_row_dequantized[DATA_BLOCK_SIZE];
    dequantize_row_fp16(f16_row, f16_row_dequantized, DATA_BLOCK_SIZE);
    print_floats("f16 Dequantized Row", f16_row_dequantized, DATA_BLOCK_SIZE);

    // q8
    printf("\n-- Row q8 Example --\n");
    Q8Row q8_row;
    quantize_row_q8(data, q8_row, DATA_BLOCK_SIZE);
    print_q8_row(q8_row);
    float q8_row_dequantized[DATA_BLOCK_SIZE];
    dequantize_row_q8(q8_row, q8_row_dequantized, DATA_BLOCK_SIZE);
    print_floats("q8 Dequantized Row", q8_row_dequantized, DATA_BLOCK_SIZE);

    // q4
    printf("\n-- Row q4 Example --\n");
    Q4Row q4_row;
    quantize_row_q4(data, q4_row, DATA_BLOCK_SIZE);
    print_q4_row(q4_row);
    float q4_row_dequantized[DATA_BLOCK_SIZE];
    dequantize_row_q4(q4_row, q4_row_dequantized, DATA_BLOCK_SIZE);
    print_floats("q4 Dequantized Row", q4_row_dequantized, DATA_BLOCK_SIZE);

    return 0;
}
