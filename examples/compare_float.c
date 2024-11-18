/**
 * @file examples/compare_float.c
 */

#include <math.h>
#include <stdio.h>

// 32-bit floating-point comparison
unsigned int compare_float(float a, float b, float threshold) {
    // Use a default threshold if none is provided
    float abs_threshold = threshold > 0.0f ? threshold : 1e-7f;

    // Absolute difference
    float diff = fabsf(a - b);

    // Handle near-zero comparisons with absolute threshold
    if (diff < abs_threshold) {
        return 1; // True
    }

    // Relative difference comparison for larger numbers
    float largest = fmaxf(fabsf(a), fabsf(b));
    if (diff < abs_threshold * largest) {
        return 1; // True
    }

    return 0; // False
}

int main() {
    float a = 0.1f * 3.0f; // Typically not exactly 0.3 due to precision
    float b = 0.3f;

    if (compare_float(a, b, 1e-7f)) {
        printf("a and b are approximately equal.\n");
    } else {
        printf("a and b are not equal.\n");
    }

    return 0;
}
