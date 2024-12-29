/**
 * Copyright © 2024 Austin Berrio
 *
 * @file float_is_close.c
 *
 * @brief Compare floating-point numbers with a given tolerance in pure C
 *
 * @note The 64-bit implementation is reverse compatible with the 32-bit
 * implementation. Whatever is true for the 64-bit implementation must then be
 * true for the 32-bit implementation.
 */

#include "float_is_close.h"

#include <math.h>
#include <stdlib.h>

// Pre-computed lookup table
static const double const tolerance_table[16]
    = {1.0,
       0.1,
       0.01,
       0.001,
       0.0001,
       0.00001,
       0.000001,
       0.0000001,
       0.00000001,
       0.000000001,
       0.0000000001,
       0.00000000001,
       0.000000000001,
       0.0000000000001,
       0.00000000000001,
       0.000000000000001};

/**
 * @brief Determine if two double-precision floating-point numbers are close
 *        within a specified tolerance.
 *
 * @param a           The first floating-point number.
 * @param b           The second floating-point number.
 * @param significand The number of significant digits to consider (must be
 *                    in the range 1 to 15 inclusive). This determines the
 *                    absolute tolerance.
 *
 * @return            True if the numbers are close within the specified
 *                    tolerance, false otherwise.
 *
 * @note The significand is clamped if it is out of range.
 * @note FIC_DOUBLE_EPSILON affects relative tolerance.
 */
bool double_is_close(double a, double b, size_t significand) {
    if (a == b) {
        return true;
    }

    if (isinf(a) || isinf(b) || isnan(a) || isnan(b)) {
        return false;
    }

    significand = FIC_CLAMP(significand, 1, 15);

    double absolute_tolerance = tolerance_table[significand];
    double relative_tolerance = FIC_DOUBLE_EPSILON * fmax(fabs(a), fabs(b));
    double difference         = fabs(a - b);

    return difference <= fmax(relative_tolerance, absolute_tolerance);
}

/**
 * @brief Determine if two single-precision floating-point numbers are close
 *        within a specified tolerance.
 *
 * @param a           The first floating-point number.
 * @param b           The second floating-point number.
 * @param significand The number of significant digits to consider (must be
 *                    in the range 1 to 7 inclusive). This determines the
 *                    absolute tolerance.
 *
 * @return            True if the numbers are close within the specified
 *                    tolerance, false otherwise.
 *
 * @note The significand is clamped if it is out of range.
 * @note FIC_SINGLE_EPSILON affects relative tolerance.
 */
bool float_is_close(float a, float b, size_t significand) {
    if (a == b) {
        return true;
    }

    if (isinf(a) || isinf(b) || isnan(a) || isnan(b)) {
        return false;
    }

    significand = FIC_CLAMP(significand, 1, 7);

    float absolute_tolerance = (float) tolerance_table[significand];
    float relative_tolerance
        = ((float) FIC_SINGLE_EPSILON) * fmaxf(fabsf(a), fabsf(b));
    float difference = fabsf(a - b);

    return difference <= fmaxf(relative_tolerance, absolute_tolerance);
}
