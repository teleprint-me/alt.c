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

void print_f16_row(const unsigned short* values, int count) {
    printf("F16 Row: ");
    for (int i = 0; i < count; ++i) {
        printf("%u ", values[i]);
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

void print_q4_row(const Q4Row row) {
    printf("Q4 Row: ");
    for (int i = 0; i < Q4_NIBBLES; ++i) {
        printf("%02x ", row[i].quant);
    }
    printf("\n");
}

int main(void) {
    int seed = 1337;
    srand(seed);

    // Example data
    float data[DATA_BLOCK_SIZE];
    for (int i = 0; i < DATA_BLOCK_SIZE; i++) {
        data[i] = ((float) rand() / (float) RAND_MAX) * 2 - 1;
    }

    // Print original data
    print_floats("Original Data", data, DATA_BLOCK_SIZE);

    // Scalar Quantization Examples

    // f16
    printf("\n-- Scalar F16 Example --\n");
    unsigned short f16 = quantize_scalar_fp16(data[0]);
    float f16_dequantized = dequantize_scalar_fp16(f16);
    printf("F16 Quantized: %04x\n", f16);
    printf("F16 Dequantized: %.6f\n", (double) f16_dequantized);

    // q8
    printf("\n-- Scalar Q8 Example --\n");
    Q8 q8 = quantize_scalar_q8(data[1]);
    float q8_dequantized = dequantize_scalar_q8(q8);
    printf("Q8 Quantized: %u\n", q8.quant);
    printf("Q8 Dequantized: %.6f\n", (double) q8_dequantized);

    // q8 underflow/overflow
    printf("\n-- Scalar Q8 Overflow --\n");
    q8 = quantize_scalar_q8(212.12345f);
    q8_dequantized = dequantize_scalar_q8(q8);
    printf("Q8 Quantized: %u\n", q8.quant);
    printf("Q8 Dequantized: %.6f\n", (double) q8_dequantized);

    // q4
    printf("\n-- Scalar Q4 Example --\n");
    Q4 q4 = quantize_scalar_q4(data[2], data[3]);
    float q4_dequantized_0 = dequantize_scalar_q4(q4, 0);
    float q4_dequantized_1 = dequantize_scalar_q4(q4, 1);
    printf("Q4 Quantized: %02x\n", q4.quant);
    printf("Q4 Dequantized (0): %.6f\n", (double) q4_dequantized_0);
    printf("Q4 Dequantized (1): %.6f\n", (double) q4_dequantized_1);

    // Row Quantization Examples

    // f16
    printf("\n-- Row F16 Example --\n");
    unsigned short f16_row[DATA_BLOCK_SIZE];
    quantize_row_fp16(data, f16_row, DATA_BLOCK_SIZE);
    print_f16_row(f16_row, DATA_BLOCK_SIZE);
    float f16_row_dequantized[DATA_BLOCK_SIZE];
    dequantize_row_fp16(f16_row, f16_row_dequantized, DATA_BLOCK_SIZE);
    print_floats("F16 Dequantized Row", f16_row_dequantized, DATA_BLOCK_SIZE);

    // q8
    printf("\n-- Row Q8 Example --\n");
    Q8Row q8_row;
    quantize_row_q8(data, q8_row, DATA_BLOCK_SIZE);
    print_q8_row(q8_row);
    float q8_row_dequantized[DATA_BLOCK_SIZE];
    dequantize_row_q8(q8_row, q8_row_dequantized, DATA_BLOCK_SIZE);
    print_floats("Q8 Dequantized Row", q8_row_dequantized, DATA_BLOCK_SIZE);

    // q4
    printf("\n-- Row Q4 Example --\n");
    Q4Row q4_row;
    quantize_row_q4(data, q4_row, DATA_BLOCK_SIZE);
    print_q4_row(q4_row);
    float q4_row_dequantized[DATA_BLOCK_SIZE];
    dequantize_row_q4(q4_row, q4_row_dequantized, DATA_BLOCK_SIZE);
    print_floats("Q4 Dequantized Row", q4_row_dequantized, DATA_BLOCK_SIZE);

    return 0;
}
