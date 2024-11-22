/**
 * @file examples/qk8.c
 */

#include <assert.h>
#include <math.h>   // For round()
#include <stdio.h>
#include <stdlib.h> // For rand() and srand()
#include <time.h>   // For seeding random number generator

#define MAX_SAMPLES 10

// Function to calculate the scaling factor based on the input range and bit precision
double scale(double max_reals, unsigned int max_bits) {
    double r_max = fabs(max_reals); // max real
    double r_min = -r_max;          // min real

    signed int z_max = (1 << (max_bits - 1)) - 1; // max integer
    signed int z_min = -(1 << (max_bits - 1));    // min integer

    return (r_max - r_min) / (double)(z_max - z_min);
}

// Function to clamp a floating-point value to a range
double clamp(double x, double min, double max) {
    if (x < min) {
        return min;
    }
    if (x > max) {
        return max;
    }
    return x;
}

// Function to quantize a floating-point value to 8-bits
signed char quantize_qint8(double x, double scalar) {
    return (signed char) round(x / scalar);
}

// Function to dequantize a quantized integer from 8-bits
double dequantize_qint8(signed char q, double scalar) {
    return q * scalar;
}

// Function to calculate quantization error
void error(double x, double x_prime, double *abs_error, double *rel_error) {
    *abs_error = fabs(x - x_prime);

    if (fabs(x) > 1e-6) { // Avoid division by zero
        *rel_error = fabs(*abs_error / x);
    } else {
        *rel_error = 0.0; // Define as 0 if the original value is effectively 0
    }
}

// Function to sample floating-point values in the range [-n, n-1]
void sampler(double *x, int max_elements, int n) {
    assert(x != NULL);
    assert(max_elements > 0);

    for (int i = 0; i < max_elements; ++i) {
        double normalized = (double) rand() / (double) RAND_MAX;
        x[i] = -n + (normalized * (2 * n - 1));
    }
}

int main() {
    // Set seed for reproducibility
    srand(1);

    // Randomly generate sampled data
    double sampled[MAX_SAMPLES];
    sampler(sampled, MAX_SAMPLES, 1);

    // User-defined quantization setup
    double max_reals = 1.0; // Magnitude of floating-point values
    unsigned int max_bits = 8; // Magnitude of integer values

    // Compute scaling factor
    double scalar = scale(max_reals, max_bits);
    printf("Scaling Factor: %f\n", scalar);

    // Define clamping bounds
    double min = -max_reals;
    double max = max_reals;

    // Initialize error accumulators
    double total_abs_error = 0.0, total_rel_error = 0.0;

    // Generate and process random samples
    printf("\nRandomly Generated Samples:\n");
    for (int i = 0; i < MAX_SAMPLES; i++) {
        double x = sampled[i];
        printf("\nInput: %f\n", x);

        // Clamp the input
        double clamped_x = clamp(x, min, max);

        // Quantize the input
        signed char q = quantize_qint8(clamped_x, scalar);
        printf("Quantized: %d\n", q);

        // Dequantize the output
        double d = dequantize_qint8(q, scalar);
        printf("Dequantized: %f\n", d);

        // Calculate the error
        double abs_error, rel_error;
        error(x, d, &abs_error, &rel_error);
        total_abs_error += abs_error;
        total_rel_error += rel_error;

        printf("Absolute Error: %f, Relative Error: %f%%\n", abs_error, rel_error * 100);
    }

    // Report average errors
    printf("\nAverage Absolute Error: %f\n", total_abs_error / MAX_SAMPLES);
    printf("Average Relative Error: %f%%\n", (total_rel_error / MAX_SAMPLES) * 100);

    return 0;
}
