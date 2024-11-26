/**
 * @file examples/q4row.c
 *
 * Demonstrates Q4 row quantization and dequantization.
 */

#include <stdlib.h> // For rand() and srand()
#include <stdio.h>  // For printf()
#include <assert.h> // For assert()
#include "data_types.h"

#define MAX_SAMPLES 10

// Function to sample floating-point values in the range [-range, range-1]
void sampler(float* x, int length, int range) {
    assert(x != NULL);
    assert(length > 0);
    assert(range > 1);

    for (int i = 0; i < length; ++i) {
        float normalized = (float) rand() / (float) RAND_MAX;
        x[i] = -(range + 1) + (normalized * ((2 * range) - 1));
    }
}

int main() {
    srand(1); // Fixed seed for reproducibility

    // Ensure sample size is even for Q4 quantization
    assert(MAX_SAMPLES % 2 == 0);

    // Generate sampled data
    float input[MAX_SAMPLES];
    sampler(input, MAX_SAMPLES, 255);

    // Step size for quantization
    int step_size = 1;

    // Allocate output buffers
    Q8Row output;
    float dequantized[MAX_SAMPLES];

    // Quantize the row
    quantize_row_q8(input, output, MAX_SAMPLES, step_size);

    // Dequantize the row
    dequantize_row_q8(output, dequantized, MAX_SAMPLES, step_size);

    // Print results
    printf("==== Q8 Row Results ===\n");
    printf("-------------------------------\n");
    printf("Index | Original   | Dequantized\n");
    printf("-------------------------------\n");
    for (int i = 0; i < MAX_SAMPLES; ++i) {
        printf("%5d | %10.2f | %12.2f\n", i, (double) input[i], (double) dequantized[i]);
    }

    return 0;
}
