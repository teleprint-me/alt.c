/**
 * Copyright © 2024 Austin Berrio
 *
 * @file test_is_float_close.c
 *
 * @brief A simple test suite for float_is_close.c in pure C
 *
 * @note The 53-bit significand precision gives from 15 to 17 significant
 *       decimal digits precision (2−53 ≈ 1.11 × 10−16).
 */

#include "float_is_close.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>

typedef struct TestCase {
    double a;
    double b;
    int64_t significand;
    bool expected;
} test_case_t;

const double expected = 0.053803;

// In comparison operations, positive infinity is larger than all values
// except itself and NaN, and negative infinity is smaller than all values
// except itself and NaN. NaN is unordered: it is not equal to, greater
// than, or less than anything, including itself.
// sauce:
// https://www.gnu.org/software/libc/manual/html_node/Infinity-and-NaN.html
const test_case_t const tests[] = {
    // precision is within a range of 6 digits
    {0.053803,         expected,         6,  true }, // equal to
    {expected,         0.053721,         6,  false}, // less than
    {expected,         0.053951,         6,  false}, // greater than
    // Negative numbers
    {-0.053803,        -expected,        6,  true }, // equal to
    {-expected,        -0.053721,        6,  false}, // greater than
    {-expected,        -0.053951,        6,  false}, // less than
    // Inf and NaN cases
    {INFINITY,         INFINITY,         6,  true }, // INF is equal to itself
    {NAN,              0.0,              6,  false}, // NaN is not equal to any object
    {NAN,              NAN,              6,  false}, // NaN is not equal to itself
    // Large numbers with significant precision
    {1e6,              1e6 + 1,          0,  false}, // no precision
    {1e-6,             1e-6 + 1e-9,      6,  true }, // Very close but small tolerance
    {123456789.123456, 123456789.123456, 15, true }, // Test large significand
    {123456789.123456, 123456789.123457, 15, false}, // Test differentiated large significand
    {1e-15,            2e-15,            15, true }  // Small numbers with large tolerance
};

const size_t number_of_tests = sizeof(tests) / sizeof(tests[0]);

int test_double_is_close(void) {
    printf("Running double_is_close tests...\n");

    // Iterate over test cases
    for (size_t i = 0; i < number_of_tests; i++) {
        bool result = double_is_close(tests[i].a, tests[i].b, tests[i].significand);

        // Output the result
        if (result == tests[i].expected) {
            printf(
                "PASS: double_is_close(%.17f, %.17f, %ld) -> Expected: %d, "
                "Got: %d\n",
                tests[i].a,
                tests[i].b,
                tests[i].significand,
                tests[i].expected,
                result
            );
        } else {
            printf(
                "FAIL: double_is_close(%.17f, %.17f, %ld) -> Expected: %d, "
                "Got: %d\n",
                tests[i].a,
                tests[i].b,
                tests[i].significand,
                tests[i].expected,
                result
            );
            // Output details for failed tests
            printf(
                "double_is_close: Expected: %.17f, Actual: %.17f, "
                "Significand: %ld\n",
                tests[i].b,
                tests[i].a,
                tests[i].significand
            );
            return 1; // tests failed: 1 on failure
        }
    }

    return 0; // tests passed: 0 on success
}

int main(void) {
    return test_double_is_close();
}
