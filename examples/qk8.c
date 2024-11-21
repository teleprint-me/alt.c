/**
 * @file examples/qk8.c
 */

#include <assert.h>
#include <math.h> // For round()
#include <stdio.h>
#include <stdlib.h> // For rand() and srand()
#include <time.h> // For seeding random number generator

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

// Function to quantize a floating-point value to Q8
int quantize(double x, double scaling_factor, double min, double max) {
    x = clamp(x, min, max); // Clamp the value
    return (int) round(x / scaling_factor);
}

// Function to dequantize a Q8 integer to floating-point
double dequantize(int q, double scaling_factor) {
    return q * scaling_factor;
}

/**
 * @brief Generates random floating-point values in the range [-n, n-1].
 *
 * This function fills the provided array with `max_elements` random values, where each value
 * is a floating-point number mapped to the range [-n, n-1]. It ensures valid inputs using
 * assertions and utilizes a uniform distribution for randomization.
 *
 * @param values       Pointer to an array to store the generated values.
 * @param max_elements Number of random values to generate.
 * @param n            The range parameter, determining the interval [-n, n-1].
 */
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

int main() {
    // Define the range for floating-point values
    double v_min_float = -1.0;
    double v_max_float = 1.0;

    // Define the range for Q8 integers
    int v_min_int = -128;
    int v_max_int = 127;

    // Calculate the scaling factor
    double scaling_factor = (v_max_float - v_min_float) / (v_max_int - v_min_int);
    printf("Scaling Factor: %f\n", scaling_factor);

    // Generate a random set of floating-point values
    srand(time(0)); // Seed for random number generation
    int num_values = 10; // Number of random values to generate
    printf("\nRandomly Generated Values:\n");
    for (int i = 0; i < num_values; i++) {
        // Generate random floating-point values between -2.0 and 2.0
        double x = ((double) rand() / RAND_MAX) * 4.0 - 2.0; // [-2.0, 2.0]
        printf("\nOriginal Floating-Point Value: %f\n", x);

        // Quantize and handle out-of-range values
        int q = quantize(x, scaling_factor, v_min_float, v_max_float);
        printf("Quantized Value (Q8): %d\n", q);

        // Dequantize back to floating-point
        double x_dequantized = dequantize(q, scaling_factor);
        printf("Dequantized Floating-Point Value: %f\n", x_dequantized);
    }

    return 0;
}
