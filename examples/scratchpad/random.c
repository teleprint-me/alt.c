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

#define MAX_ELEMENTS 10  ///< Maximum number of elements to generate

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

int main(int argc, char** argv) {
    // Default parameters
    int n = 5;        // Default range
    unsigned int seed = (unsigned int) time(NULL);  // Default seed

    // No arguments given
    if (argc == 1) {
        fprintf(stdout, "Usage: %s <optional range> <optional seed>\n", argv[0]);
    }

    // 1 argument given
    if (argc > 1) {
        n = atoi(argv[1]);  // First argument: range parameter
        if (n <= 0) {
            fprintf(stderr, "Error: Range parameter n must be positive.\n");
            return 1;
        }
    }

    // 2 arguments given
    if (argc > 2) {
        seed = (unsigned int) atoi(argv[2]);
        if (seed <= 0) {  // Correct the condition to validate seed
            fprintf(stderr, "Error: srand() parameter seed must be positive.\n");
            return 1;
        }
    }

    fprintf(stdout, "Using: %s where range = [-%d, %d - 1], seed = %u\n", argv[0], n, n, seed);

    // Seed the random number generator
    srand(seed);

    // Generate random values
    float values[MAX_ELEMENTS];
    generate_random_values(values, MAX_ELEMENTS, n);

    // Print the generated values
    for (int i = 0; i < MAX_ELEMENTS; ++i) {
        printf("%.7f\n", (double) values[i]);
    }

    return 0;
}
