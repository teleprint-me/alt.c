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

// Data type management

const DataType* data_type_get(DataTypeId id) {
    // Bounds checking to avoid invalid access
    if (id >= TYPE_COUNT) {
        return NULL; // Invalid type
    }
    return &TYPES[id];
}

uint32_t data_type_size(DataTypeId id) {
    const DataType* type = data_type_get(id);
    return type ? type->size : 0;
}

const char* data_type_name(DataTypeId id) {
    const DataType* type = data_type_get(id);
    return type ? type->name : "Unknown";
}

// Scalar Conversions

// 32-bit encoding and decoding
uint32_t encode_scalar_fp32(float value) {
    FloatBits raw;
    raw.value = value;
    return raw.bits;
}

float decode_scalar_fp32(uint32_t bits) {
    FloatBits raw;
    raw.bits = bits;
    return raw.value;
}

// Half-precision floating-point quantization
uint16_t quantize_scalar_fp16(float value) {
    const float scale_to_inf = 0x1.0p+112f;
    const float scale_to_zero = 0x1.0p-110f;
    float base = (fabsf(value) * scale_to_inf) * scale_to_zero;

    const uint32_t w = encode_scalar_fp32(value);
    const uint32_t shl1_w = w + w;
    const uint32_t sign = w & 0x80000000;
    uint32_t bias = shl1_w & 0xFF000000;

    if (bias < 0x71000000) {
        bias = 0x71000000;
    }

    base = decode_scalar_fp32((bias >> 1) + 0x07800000) + base;
    const uint32_t bits = encode_scalar_fp32(base);
    const uint32_t exp_bits = (bits >> 13) & 0x00007C00;
    const uint32_t mantissa_bits = bits & 0x00000FFF;
    const uint32_t nonsign = exp_bits + mantissa_bits;
    return (sign >> 16) | (shl1_w > 0xFF000000 ? 0x7E00 : nonsign);
}

float dequantize_scalar_fp16(uint16_t bits) {
    const uint32_t w = (uint32_t) bits << 16;
    const uint32_t sign = w & 0x80000000;
    const uint32_t two_w = w + w;

    const uint32_t exp_offset = 0xE0 << 23;
    const float exp_scale = 0x1.0p-112f;
    const float normalized_value = decode_scalar_fp32((two_w >> 4) + exp_offset) * exp_scale;

    const uint32_t magic_mask = 0x7E000000;
    const float magic_bias = 0.5f;
    const float denormalized_value = decode_scalar_fp32((two_w >> 17) | magic_mask) - magic_bias;

    const uint32_t denormalized_cutoff = 1 << 27;
    const uint32_t result = sign
                            | (two_w < denormalized_cutoff ? encode_scalar_fp32(denormalized_value)
                                                           : encode_scalar_fp32(normalized_value));
    return decode_scalar_fp32(result);
}

// 8-bit quantization with residual baking
Q8 quantize_scalar_q8(float value) {
    Q8 q8;

    // Define integer domain
    uint32_t z_domain = 255;
    // Reflect and compute effective real domain
    float r_domain = fabsf(value);

    // Special case for zero
    if (r_domain == 0.0f) {
        q8.scalar = quantize_scalar_fp16(1.0f);
        q8.bits = 0;
        return q8;
    }

    // Calculate squeezing ratio
    float alpha = (r_domain > z_domain) ? z_domain / r_domain : 1.0f;
    // Calculate the base step size
    float step_size = r_domain / z_domain; // Decoupled from scalar
    // Quantize the value using the base step size (exponent/mantissa?)
    uint8_t bits = roundf(value / step_size);
    // Calculate the residual precision (bias?)
    float residual = (value - (bits * step_size));

    // Calculate the scalar based on the squeezed range
    float scalar = step_size * alpha + residual;

    // Quantize to scalar to half-precision
    q8.scalar = quantize_scalar_fp16(scalar);
    // Quantize the value
    q8.bits = bits;

    return q8;
}

// Dequantize the value
float dequantize_scalar_q8(Q8 q8) {
    // Dequantize the scalar value
    float scalar = dequantize_scalar_fp16(q8.scalar);
    // Scale the bits proportionally back to original float
    return (float) (q8.bits * scalar);
}

// 4-bit integer quantization (packed)
Q4 quantize_scalar_q4(float a, float b) {
    Q4 q4;

    // Define integer domain for Q4
    uint32_t z_domain = 7; // [-8, 7] for signed
    float max_abs = fmaxf(fabsf(a), fabsf(b));

    // Special case for zero
    if (max_abs == 0.0f) {
        q4.scalar = quantize_scalar_fp16(1.0f);
        q4.bits = 0;
        return q4;
    }

    // Calculate step size and combined scalar
    float step_size = max_abs / z_domain;
    q4.scalar = quantize_scalar_fp16(step_size);

    // Quantize each value
    int8_t q_a = (int8_t) roundf(a / step_size);
    int8_t q_b = (int8_t) roundf(b / step_size);

    // Clamp to Q4 range
    q_a = fmaxf(fminf(q_a, 7), -8);
    q_b = fmaxf(fminf(q_b, 7), -8);

    // Pack the two 4-bit values into one byte
    q4.bits = ((q_a & 0x0F) << 4) | (q_b & 0x0F);

    return q4;
}

// Dequantize packed Q4 values

// By-Value API: Use for single-value dequantization.
float dequantize_scalar_q4_index(Q4 q4, uint32_t index) {
    // Dequantize the scalar
    float step_size = dequantize_scalar_fp16(q4.scalar);

    // Extract the 4-bit value
    int8_t q_value = (index == 0) ? (q4.bits >> 4) & 0x0F : q4.bits & 0x0F;

    // Convert to signed 4-bit
    if (q_value & 0x08) q_value |= 0xF0; // Sign extend if negative

    // Convert back to float
    return (float)(q_value * step_size);
}

// By-Reference API: Use for batch processing or when both values are typically needed.
void dequantize_scalar_q4_reference(Q4 q4, float* a, float* b) {
    // Dequantize the scalar
    float scalar = dequantize_scalar_fp16(q4.scalar);

    // Extract 4-bit values
    int8_t qa = (q4.bits >> 4) & 0x0F; // upper nibble
    if (qa & 0x08) qa |= 0xF0;         // Sign-extend if negative
    int8_t qb = q4.bits & 0x0F;        // lower nibble
    if (qb & 0x08) qb |= 0xF0;         // Sign-extend if negative

    // Convert back to float
    *a = (float)(qa * scalar);
    *b = (float)(qb * scalar);
}

// Vector Conversions (1D arrays)

// Half-precision floating-point quantization
void quantize_row_fp16(const float* input, uint16_t* output, uint32_t length, uint32_t step_size) {
    assert(input != NULL);
    assert(output != NULL);
    assert(length > 0);
    assert(step_size > 0);

    for (uint32_t i = 0, j = 0; i < length; i += step_size, ++j) {
        output[j] = quantize_scalar_fp16(input[i]);
    }
}

void dequantize_row_fp16(const uint16_t* input, float* output, uint32_t length, uint32_t step_size) {
    assert(input != NULL);
    assert(output != NULL);
    assert(length > 0);
    assert(step_size > 0);

    for (uint32_t i = 0, j = 0; i < length; i += step_size, ++j) {
        output[i] = dequantize_scalar_fp16(input[j]);
    }
}

// 8-bit integer quantization
void quantize_row_q8(const float* input, Q8Row output, uint32_t length, uint32_t step_size) {
    assert(input != NULL);
    assert(output != NULL);
    assert(length > 0);
    assert(step_size > 0);

    for (uint32_t i = 0, j = 0; i < length; i += step_size, ++j) {
        output[j] = quantize_scalar_q8(input[i]);
    }
}

void dequantize_row_q8(const Q8Row input, float* output, uint32_t length, uint32_t step_size) {
    assert(input != NULL);
    assert(output != NULL);
    assert(length > 0);
    assert(step_size > 0);

    for (uint32_t i = 0, j = 0; i < length; i += step_size, ++j) {
        output[i] = dequantize_scalar_q8(input[j]);
    }
}

// 4-bit integer quantization
void quantize_row_q4(const float* input, Q4Row output, uint32_t length, uint32_t step_size) {
    assert(input != NULL);
    assert(output != NULL);
    assert(length > 0);
    assert(step_size > 0);
    assert((length / step_size) % 2 == 0); // Ensure input size is even for Q4

    for (uint32_t i = 0, j = 0; i < length; i += 2 * step_size, ++j) {
        output[j] = quantize_scalar_q4(input[i], input[i + step_size]);
    }
}

void dequantize_row_q4(const Q4Row input, float* output, uint32_t length, uint32_t step_size) {
    assert(input != NULL);
    assert(output != NULL);
    assert(length > 0);
    assert(step_size > 0);
    assert((length / step_size) % 2 == 0); // Ensure output size is even for Q4

    for (uint32_t i = 0, j = 0; i < length; i += 2 * step_size, ++j) {
        output[i] = dequantize_scalar_q4_index(input[j], 0);      // Lower nibble
        output[i + step_size] = dequantize_scalar_q4_index(input[j], 1); // Upper nibble
    }
}
