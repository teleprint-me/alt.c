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

// 8-bit integer quantization
Q8 quantize_scalar_q8(float value) {
    Q8 q8;

    // Set the range for the integer domain
    int z_max = 127;
    int z_min = -128;

    // Clamp the real input value to the integer domain
    float clamped = CLAMP(value, z_min, z_max);

    // Calculate the dynamic range for the real domain
    float r_max = fabsf(clamped); // Use the absolute value for max
    float r_min = -r_max;         // Reflect the max as min in the negative domain

    // Calculate the scaling factor and preserve the sign
    q8.scalar = (r_max - r_min) / (z_max - z_min);
    q8.scalar = (value >= 0) ? q8.scalar : -q8.scalar;

    // Quantize the value
    q8.bits = (uint8_t) roundf(clamped / q8.scalar);

    return q8;
}

// Reconstruct the real value using the scalar
float dequantize_scalar_q8(Q8 q8) {
    return (float) q8.bits * q8.scalar;
}

// 4-bit integer quantization
Q4 quantize_scalar_q4(float a, float b) {
    Q4 q4;

    // Set the range for the integer domain
    int z_max = 7;
    int z_min = -8;

    // Clamp the real input values to the integer domain
    float a_clamped = CLAMP(a, z_min, z_max);
    float b_clamped = CLAMP(b, z_min, z_max);

    // Calculate the dynamic range for the real domain
    float a_max = fabsf(a_clamped);
    float a_min = -a_max;
    float a_scalar = (a_max - a_min) / (z_max - z_min);
    float b_max = fabsf(b_clamped);
    float b_min = fabsf(b_clamped);
    float b_scalar = (b_max - b_min) / (z_max - z_min);

    // Calculate the scaling factor
    q4.scalar = (a_scalar + b_scalar) / 2; // Use the mean of the scalars

    // Quantize by scaling and rounding
    int8_t quant1 = (int8_t) roundf(a_clamped / q4.scalar);
    int8_t quant2 = (int8_t) roundf(b_clamped / q4.scalar);

    // Pack two quantized values into a single byte
    q4.bits = ((uint8_t) quant2 << 4) | ((uint8_t) quant1 & 0x0F);

    return q4;
}

float dequantize_scalar_q4(Q4 q4, int index) {
    int8_t bits;

    if (index == 0) { // Lower nibble
        bits = q4.bits & 0x0F;
        if (bits & 0x08) {
            bits -= 16; // Sign extension for 4-bit negative values
        }
    } else { // Upper nibble
        bits = (q4.bits >> 4) & 0x0F;
        if (bits & 0x08) {
            bits -= 16;
        }
    }

    return bits * q4.scalar;
}

// Vector Conversions (1D arrays)

// Half-precision floating-point quantization
void quantize_row_fp16(const float* input, uint16_t* output, int count) {
    assert(input != NULL);
    assert(output != NULL);

    for (int i = 0; i < count; ++i) {
        output[i] = quantize_scalar_fp16(input[i]);
    }
}

void dequantize_row_fp16(const uint16_t* input, float* output, int count) {
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
