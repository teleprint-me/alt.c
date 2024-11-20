/**
 * @file random.c
 *
 * @brief Demonstrates weight initialization in machine learning by generating random
 * numbers within a specified range. This program normalizes random numbers between 0 and 1,
 * maps them to a symmetric range [-n, n-1], and prints the results. The random number
 * generator is seeded for reproducibility.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_ELEMENTS 10 ///< Maximum number of elements to generate

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
    int n = 5; // Define the range parameter
    float values[MAX_ELEMENTS];

    // Seed the random number generator with the current time
    srand(1337); // Use a discrete deterministic value for reproducibility

    // Generate random values in the range [-n, n-1]
    generate_random_values(values, MAX_ELEMENTS, n);

    // Print the generated values
    printf("Random values in the range [-%d, %d-1]:\n", n, n);
    for (int i = 0; i < MAX_ELEMENTS; ++i) {
        printf("%.7f\n", (double) values[i]);
    }

    return 0;
}
