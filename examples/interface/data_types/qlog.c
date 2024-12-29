/**
 * @file examples/qlog.c
 */

#include <math.h>
#include <stdint.h>
#include <stdio.h>

// Logarithmic quantization parameters
#define LOG_BASE 2.0
#define EPSILON 1e-6

// Encode value using logarithmic quantization
uint8_t encode_log(float value) {
    int sign = (value < 0) ? 1 : 0;
    value = fabs(value);

    // Logarithmic scaling
    float log_value = log2(value + EPSILON);

    // Map to range [0, 255] (for 8-bit quantization)
    uint8_t encoded = (uint8_t) (log_value * 16); // Scale log_value to fit range

    return (sign << 7) | encoded; // Pack sign and magnitude
}

// Decode value back to floating-point
float decode_log(uint8_t encoded) {
    int sign = (encoded >> 7) & 0x01;
    uint8_t magnitude = encoded & 0x7F;

    // Rescale log value and compute exponent
    float log_value = magnitude / 16.0;
    float value = pow(2.0, log_value) - EPSILON;

    return sign ? -value : value;
}
