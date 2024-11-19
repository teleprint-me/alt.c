/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/data_types.c
 *
 * @brief API for handling numeric data types and conversions.
 *
 * Focused on:
 * - Single and half-precision floating-point.
 * - 8-bit and 4-bit quantized integers.
 * - Minimal dependencies and consistent design.
 */

#include "data_types.h"

#include <assert.h>
#include <math.h>
#include <string.h>

// Scalar Conversions

// Floating-point encoding and decoding
unsigned int encode_float32_to_bits(float value) {
    FloatBits raw;
    raw.value = value;
    return raw.bits;
}

float decode_bits_to_float32(unsigned int bits) {
    FloatBits raw;
    raw.bits = bits;
    return raw.value;
}

// Half-precision floating-point quantization
unsigned short quantize_scalar_fp16(float value) {
    float base = (fabsf(value) * F16_SCALE_TO_INF) * F16_SCALE_TO_ZERO;

    const unsigned int w = encode_float32_to_bits(value);
    const unsigned int shl1_w = w + w; // Double the exponent
    const unsigned int sign = w & F16_SIGN_MASK; // Extract the sign bit

    unsigned int bias = shl1_w & F32_EXPONENT_MASK; // Extract exponent
    if (bias < F16_SMALLEST_EXPONENT) { // Clamp small exponents
        bias = F16_SMALLEST_EXPONENT;
    }

    base = decode_bits_to_float32((bias >> 1) + F16_BIAS_ADJUSTMENT) + base;

    const unsigned int bits = encode_float32_to_bits(base);
    const unsigned int exp_bits = (bits >> 13) & F16_EXP_MASK; // f16 exponent
    const unsigned int mantissa_bits = bits & F16_MANTISSA_MASK; // f16 mantissa

    const unsigned int nonsign = exp_bits + mantissa_bits;
    return (sign >> 16) | (shl1_w > F32_EXPONENT_MASK ? F16_INFINITY : nonsign);
}

float dequantize_scalar_fp16(unsigned short bits) {
    const unsigned int w = (unsigned int) bits << 16;   // Convert f16 to f32
    const unsigned int sign = w & F16_SIGN_MASK;          // Extract sign bit
    const unsigned int two_w = w + w;                     // Double exponent and mantissa

    // Handle normalized values
    const float normalized_value
        = decode_bits_to_float32((two_w >> 4) + F16_EXP_OFFSET) * F16_EXP_SCALE;

    // Handle denormalized values
    const float denormalized_value
        = decode_bits_to_float32((two_w >> 17) | F16_MAGIC_MASK) - F16_MAGIC_BIAS;

    // Determine if value is denormalized
    const unsigned int result
        = sign
          | (two_w < F16_DENORMALIZED_CUTOFF
                 ? encode_float32_to_bits(denormalized_value)
                 : encode_float32_to_bits(normalized_value));

    return decode_bits_to_float32(result);
}

// 8-bit integer quantization
Q8 quantize_scalar_q8(float value) {
    Q8 q8;

    // Determine delta based on absolute value
    q8.delta = fabsf(value) / 127.0f;
    q8.min = -127.0f * q8.delta;
    q8.max = 127.0f * q8.delta;

    // Clamp and quantize
    float clamped = CLAMP(value, q8.min, q8.max);
    q8.scalar = (unsigned char) roundf(clamped / q8.delta);

    return q8;
}

float dequantize_scalar_q8(Q8 q8) {
    return q8.scalar * q8.delta;
}

// 4-bit integer quantization
Q4 quantize_scalar_q4(float value1, float value2) {
    Q4 q4;

    // Determine delta using the larger absolute value
    q4.delta = fmaxf(fabsf(value1), fabsf(value2)) / 7.0f;
    q4.min = -7.0f * q4.delta;
    q4.max = 7.0f * q4.delta;

    // Clamp and quantize the two values
    float clamped1 = CLAMP(value1, q4.min, q4.max);
    float clamped2 = CLAMP(value2, q4.min, q4.max);

    signed char quant1 = (signed char) roundf(clamped1 / q4.delta);
    signed char quant2 = (signed char) roundf(clamped2 / q4.delta);

    // Pack two quantized values into a single byte
    q4.scalar = (quant2 << 4) | (quant1 & 0x0F);
    return q4;
}

float dequantize_scalar_q4(Q4 q4, int index) {
    signed char quant;

    if (index == 0) { // Lower nibble
        quant = q4.scalar & 0x0F;
        if (quant & 0x08) {
            quant -= 16; // Sign extension for 4-bit negative values
        }
    } else { // Upper nibble
        quant = (q4.scalar >> 4) & 0x0F;
        if (quant & 0x08) {
            quant -= 16;
        }
    }

    return quant * q4.delta;
}

// Vector Conversions (1D arrays)

// Half-precision floating-point quantization
void quantize_row_fp16(const float* input, unsigned short* output, int count) {
    assert(input != NULL);
    assert(output != NULL);

    for (int i = 0; i < count; ++i) {
        output[i] = quantize_scalar_fp16(input[i]);
    }
}

void dequantize_row_fp16(const unsigned short* input, float* output, int count) {
    assert(input != NULL);
    assert(output != NULL);

    for (int i = 0; i < count; ++i) {
        output[i] = dequantize_scalar_fp16(input[i]);
    }
}

// 8-bit integer quantization
void quantize_row_q8(const float* input, Q8Row output, int count) {
    assert(input != NULL);
    assert(output != NULL);
    assert(count % Q8_ELEMENTS == 0);

    for (int i = 0; i < count / Q8_ELEMENTS; ++i) {
        for (int j = 0; j < Q8_ELEMENTS; ++j) {
            output[i * Q8_ELEMENTS + j] = quantize_scalar_q8(input[i * Q8_ELEMENTS + j]);
        }
    }
}

void dequantize_row_q8(const Q8Row input, float* output, int count) {
    assert(input != NULL);
    assert(output != NULL);
    assert(count % Q8_ELEMENTS == 0);

    for (int i = 0; i < count / Q8_ELEMENTS; ++i) {
        for (int j = 0; j < Q8_ELEMENTS; ++j) {
            output[i * Q8_ELEMENTS + j] = dequantize_scalar_q8(input[i * Q8_ELEMENTS + j]);
        }
    }
}

// 4-bit integer quantization
void quantize_row_q4(const float* input, Q4Row output, int count) {
    assert(input != NULL);
    assert(output != NULL);
    assert(count % Q4_NIBBLES == 0); // Ensure input size is even

    for (int i = 0; i < count / 2; ++i) {
        output[i] = quantize_scalar_q4(input[2 * i], input[2 * i + 1]);
    }
}

void dequantize_row_q4(const Q4Row input, float* output, int count) {
    assert(input != NULL);
    assert(output != NULL);
    assert(count % Q4_NIBBLES == 0); // Ensure output size is even

    for (int i = 0; i < count / 2; ++i) {
        output[2 * i] = dequantize_scalar_q4(input[i], 0); // Lower nibble
        output[2 * i + 1] = dequantize_scalar_q4(input[i], 1); // Upper nibble
    }
}
