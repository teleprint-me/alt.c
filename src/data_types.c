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
#include <wchar.h>

uint32_t data_type_size(DataType type) {
    switch(type) {
        // Floating-point types
        case TYPE_DOUBLE:       return sizeof(double);
        case TYPE_FLOAT:        return sizeof(float);
        case TYPE_FLOAT16:      return sizeof(uint16_t); // IEEE-754 half-precision
        case TYPE_BFLOAT16:     return sizeof(uint16_t); // Brain floating-point 16-bit

        // Signed integers
        case TYPE_INT64:        return sizeof(int64_t);
        case TYPE_INT32:        return sizeof(int32_t);
        case TYPE_INT16:        return sizeof(int16_t);
        case TYPE_INT8:         return sizeof(int8_t); // Unpacked 8-bits
        case TYPE_INT4:         return sizeof(int8_t); // Packed 4-bits

        // Unsigned integers
        case TYPE_UINT64:       return sizeof(uint64_t);
        case TYPE_UINT32:       return sizeof(uint32_t);
        case TYPE_UINT16:       return sizeof(uint16_t);
        case TYPE_UINT8:        return sizeof(uint8_t); // Unpacked 8-bits
        case TYPE_UINT4:        return sizeof(uint8_t); // Packed 4-bits

        // Boolean
        case TYPE_BOOL:         return sizeof(uint8_t); // Typically stored as 1 byte

        // Character types
        case TYPE_CHAR:         return sizeof(char);
        case TYPE_WCHAR:        return sizeof(wchar_t);

        // Complex numbers
        case TYPE_COMPLEX_FLOAT:  return 2 * sizeof(float);
        case TYPE_COMPLEX_DOUBLE: return 2 * sizeof(double);

        // Custom and unsupported types
        case TYPE_CUSTOM:
        default:
            return 0; // Unsupported or unknown type
    }
}

// Scalar Conversions

// 64-bit encoding and decoding
uint64_t encode_scalar_fp64(double value) {
    DoubleBits raw;
    raw.value = value;
    return raw.bits;
}

double decode_scalar_fp64(uint64_t bits) {
    DoubleBits raw;
    raw.bits = bits;
    return raw.value;
}

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
    const uint32_t result
        = sign
          | (two_w < denormalized_cutoff ? encode_scalar_fp32(denormalized_value)
                                         : encode_scalar_fp32(normalized_value));
    return decode_scalar_fp32(result);
}

// 8-bit integer quantization
Q8 quantize_scalar_q8(float value) {
    Q8 q8;

    // Fixed delta based on the full q8 range
    q8.scalar = 1.0f / 127.0f; // Scaling factor for the full range [-127, 127]
    q8.min = -127.0f;
    q8.max = 127.0f;

    // Clamp the input value to the quantizable range
    float clamped = CLAMP(value, q8.min, q8.max);

    // Quantize by scaling and rounding
    int8_t quant = (int8_t) roundf(clamped / q8.scalar);

    // Store the quantized value
    q8.quant = (unsigned char) quant + q8.max; // Shift the range [0, 255]

    return q8;
}

float dequantize_scalar_q8(Q8 q8) {
    return q8.scalar * (q8.quant + q8.min); // Shift the range [-127, 127]
}

// 4-bit integer quantization
Q4 quantize_scalar_q4(float a, float b) {
    Q4 q4;

    q4.min = -7.0f;
    q4.max = 7.0f;

    // Fixed-scalar for the full Q4 range
    q4.scalar = 1.0f / q4.max; // Scaling factor for the full range [-7, 7]

    // Clamp the two values
    float clamped1 = CLAMP(a, q4.min, q4.max);
    float clamped2 = CLAMP(b, q4.min, q4.max);

    // Quantize by scaling and rounding
    int8_t quant1 = (int8_t) roundf(clamped1 / q4.scalar);
    int8_t quant2 = (int8_t) roundf(clamped2 / q4.scalar);

    // Pack two quantized values into a single byte
    q4.quant = ((unsigned char) quant2 << 4) | ((unsigned char) quant1 & 0x0F);

    return q4;
}

float dequantize_scalar_q4(Q4 q4, int index) {
    int8_t quant;

    if (index == 0) { // Lower nibble
        quant = q4.quant & 0x0F;
        if (quant & 0x08) {
            quant -= 16; // Sign extension for 4-bit negative values
        }
    } else { // Upper nibble
        quant = (q4.quant >> 4) & 0x0F;
        if (quant & 0x08) {
            quant -= 16;
        }
    }

    return q4.scalar * quant;
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
