/**
 * Copyright © 2024 Austin Berrio
 *
 * @file float_is_close.h
 *
 * @brief Compare floating-point numbers with a given tolerance in pure C
 *
 * @note FIC is an abbreviation of FLOAT_IS_CLOSE
 *
 * @note see 1.2: Epsilon-Delta Definition of a Limit for details
 * https://math.libretexts.org/Bookshelves/Calculus/Calculus_3e_(Apex)
 */

#ifndef FLOAT_IS_CLOSE_H
#define FLOAT_IS_CLOSE_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief The smallest difference between two distinct double-precision
 *        floating-point numbers (optional).
 *
 * @note For 64-bit doubles with a 53-bit significand, this is approximately
 *       10**-15.
 */
#ifndef FIC_DOUBLE_EPSILON
    #define FIC_DOUBLE_EPSILON 1e-15
#endif

/**
 * @brief The smallest difference between two distinct single-precision
 *        floating-point numbers (optional).
 *
 * @note For 32-bit floats with a 24-bit significand, this is approximately
 *       10**-7.
 */
#ifndef FIC_SINGLE_EPSILON
    #define FIC_SINGLE_EPSILON 1e-7
#endif

/**
 * @brief Get the minimum of two values.
 *
 * @note This macro compares two arguments and returns the smaller one.
 */
#define FIC_MIN(a, b)                  ((a) < (b) ? (a) : (b))

/**
 * @brief Get the maximum of two values.
 *
 * @note This macro compares two arguments and returns the larger one.
 */
#define FIC_MAX(a, b)                  ((a) > (b) ? (a) : (b))

/**
 * @brief Clamp a value to a specified range.
 *
 * @param value The input value to bind
 * @param lower The lower boundary
 * @param upper The upper boundary
 *
 * @note This macro ensures that a value stays within a given range by
 *       clamping it to the specified minimum and maximum values.
 */
#define FIC_CLAMP(value, lower, upper) (FIC_MAX(lower, FIC_MIN(value, upper)))

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
bool double_is_close(double a, double b, size_t significand);

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
bool float_is_close(float a, float b, size_t significand);

#endif // FLOAT_IS_CLOSE_H
