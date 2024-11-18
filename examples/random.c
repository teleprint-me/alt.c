/**
 * @file examples/random.c
 *
 * @brief This program demonstrates weight initialization in machine learning by generating random
 * numbers between 0 and 1, normalizing them, and ensuring they fall within a specified range using
 * the interval function. The program uses the srand function to seed the random number generator
 * and the rand function to generate random numbers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void initialize(void) {
    long z = time(NULL); // use current time as seed
    printf("initialize: %d\n", z);
    srand(z); // seed the rng using current time
    printf("srand: seeded\n");
}

float normalize(void) {
    long z = rand(); // return a number between z and RAND_MAX
    printf("rand: %d\n", z);
    float norm = (float) z / RAND_MAX;
    printf("normalize: %f\n", norm);
    return norm;
}

void interval(float n) {
    float i = n * 2 - 1; // bind to interval [-1, 1]
    printf("interval: %f\n", i);
}

int main(void) {
    int max_outputs = 10;
    initialize();
    for (int i = 0; i < max_outputs; i++) {
        float norm = normalize();
        interval(norm);
    }
    return 0;
}
