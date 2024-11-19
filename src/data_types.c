/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/data_types.c
 *
 * @brief API for handling various numeric types and conversions, currently
 *        focused on 32-bit floating-point (float) and integer (int32_t)
 *        representations. Future extensions may include 16-bit and 8-bit
 *        formats for digital signal processing.
 *
 * Only pure C is used with minimal dependencies on external libraries.
 *
 * - Keep the interface minimal and focused.
 * - Avoid generics; stick to a single base type (float, int32) for now.
 * - Isolate conversion logic into a utility file/module.
 * - Maintain clean and comprehensible separation between different components.
 */

#include "data_types.h"

#include <assert.h>
#include <math.h>
#include <string.h>

/**
 * @brief Encodes a given floating-point value into its corresponding 32-bit integer
 *        representation (IEEE-754 format).
 *
 * @param[in] value The floating-point value to encode.
 *
 * @return The resulting encoded 32-bit integer representation of the input value.
 */
unsigned int data_encode_float32(float value) {
    UnionType data;
    data.value = value;
    return data.bits;
}

/**
 * @brief Decodes a 32-bit integer representation into its corresponding floating-point value.
 *
 * @param[in] bits The encoded 32-bit integer bit representation of the floating-point number.
 *
 * @return The decoded 32-bit floating-point value.
 */
float data_decode_float32(unsigned int bits) {
    UnionType data;
    data.bits = bits;
    return data.value;
}

/**
 * @brief Converts a 16-bit floating-point (half precision) value to a 32-bit floating-point value.
 *
 * @param[in] fp16_val The 16-bit floating-point value to convert.
 *
 * @return The converted 32-bit floating-point value.
 */
float data_fp16_to_fp32(unsigned short fp16_val) {
    const unsigned int w = (unsigned int) fp16_val << 16;
    const unsigned int sign = w & 0x80000000;
    const unsigned int two_w = w + w;

    const unsigned int exp_offset = 0xE0 << 23;
    const float exp_scale = 0x1.0p-112f;
    const float normalized_value = data_decode_float32((two_w >> 4) + exp_offset) * exp_scale;

    const unsigned int magic_mask = 0x7E000000;
    const float magic_bias = 0.5f;
    const float denormalized_value = data_decode_float32((two_w >> 17) | magic_mask) - magic_bias;

    const unsigned int denormalized_cutoff = 1 << 27;
    const unsigned int result
        = sign
          | (two_w < denormalized_cutoff ? data_encode_float32(denormalized_value)
                                         : data_encode_float32(normalized_value));
    return data_decode_float32(result);
}

/**
 * @brief Converts a 32-bit floating-point value to a 16-bit floating-point (half precision) value.
 *
 * @param[in] fp32_val The 32-bit floating-point value to convert.
 *
 * @return The converted 16-bit floating-point value.
 */
unsigned short data_fp32_to_fp16(float fp32_val) {
    const float scale_to_inf = 0x1.0p+112f;
    const float scale_to_zero = 0x1.0p-110f;
    float base = (fabsf(fp32_val) * scale_to_inf) * scale_to_zero;

    const unsigned int w = data_encode_float32(fp32_val);
    const unsigned int shl1_w = w + w;
    const unsigned int sign = w & 0x80000000;
    unsigned int bias = shl1_w & 0xFF000000;
    if (bias < 0x71000000) {
        bias = 0x71000000;
    }

    base = data_decode_float32((bias >> 1) + 0x07800000) + base;
    const unsigned int bits = data_encode_float32(base);
    const unsigned int exp_bits = (bits >> 13) & 0x00007C00;
    const unsigned int mantissa_bits = bits & 0x00000FFF;
    const unsigned int nonsign = exp_bits + mantissa_bits;
    return (sign >> 16) | (shl1_w > 0xFF000000 ? 0x7E00 : nonsign);
}

/**
 * @brief Quantizes a row of float values into 8-bit values (q8).
 * @param input_vals Pointer to the input array of floats to be quantized.
 * @param output_block Pointer to the output quantized block.
 * @param num_elements Number of elements in the input array (must be a multiple of QK8_ELEMENTS).
 */
void data_quantize_row_q8_0(
    const float* restrict input_vals, void* restrict output_block, int num_elements
) {
    assert(num_elements % QK8_ELEMENTS == 0);

    block_q8_0* restrict out_block = (block_q8_0*) output_block;
    int num_blocks = num_elements / QK8_ELEMENTS;

    for (int i = 0; i < num_blocks; i++) {
        float max_abs_val = 0.0f;

        // Find the maximum absolute value in the block for delta calculation
        for (int j = 0; j < QK8_ELEMENTS; j++) {
            max_abs_val = fmaxf(max_abs_val, fabsf(input_vals[i * QK8_ELEMENTS + j]));
        }

        // Calculate delta and its reciprocal
        float delta = max_abs_val / 127.0f; // Scale to fit in 8-bit signed range [-128, 127]
        float inv_delta = (delta != 0.0f) ? 1.0f / delta : 0.0f;

        out_block[i].delta = delta;

        // Quantize each value
        for (int j = 0; j < QK8_ELEMENTS; j++) {
            float scaled_val = input_vals[i * QK8_ELEMENTS + j] * inv_delta;
            out_block[i].elements[j]
                = (signed char) roundf(scaled_val); // Convert to nearest integer in [-128, 127]
        }
    }
}

/**
 * @brief Dequantizes a row of 8-bit quantized values back into float values.
 * @param input_block Pointer to the input quantized block.
 * @param output_vals Pointer to the output array of dequantized floats.
 * @param num_elements Number of elements to be dequantized (must be a multiple of QK8_ELEMENTS).
 */
void data_dequantize_row_q8_0(
    const void* restrict input_block, float* restrict output_vals, int num_elements
) {
    assert(num_elements % QK8_ELEMENTS == 0);

    const block_q8_0* restrict in_block = (const block_q8_0*) input_block;
    int num_blocks = num_elements / QK8_ELEMENTS;

    for (int i = 0; i < num_blocks; i++) {
        float delta = in_block[i].delta;

        // Dequantize each element
        for (int j = 0; j < QK8_ELEMENTS; j++) {
            output_vals[i * QK8_ELEMENTS + j] = in_block[i].elements[j] * delta;
        }
    }
}

/**
 * @brief Quantizes a row of float values into 4-bit values (q4).
 * @param input_vals Pointer to the input array of floats to be quantized.
 * @param output_block Pointer to the output quantized block.
 * @param num_elements Number of elements in the input array (must be a multiple of QK4_NIBBLES * 2).
 */
void data_quantize_row_q4_0(
    const float* restrict input_vals, void* restrict output_block, int num_elements
) {
    assert(num_elements % (QK4_NIBBLES * 2) == 0);

    block_q4_0* restrict out_block = (block_q4_0*) output_block;
    int num_blocks = num_elements / (QK4_NIBBLES * 2);

    // Temporary array for packed signed 4-bit quantized values
    signed char quantized_nibbles[QK4_NIBBLES];

    for (int i = 0; i < num_blocks; i++) {
        float max_abs_val = 0.0f;

        // Find max absolute value in the block to calculate delta
        for (int j = 0; j < QK4_NIBBLES * 2; j++) {
            max_abs_val = fmaxf(max_abs_val, fabsf(input_vals[i * QK4_NIBBLES * 2 + j]));
        }

        // Calculate delta to scale values into signed 4-bit range [-8, 7]
        float delta = max_abs_val / 7.0f;

        // inv_delta is used for efficient scaling during quantization
        float inv_delta = (delta != 0.0f) ? 1.0f / delta : 0.0f;

        out_block[i].delta = delta;

        // Quantize each pair of values into signed 4-bit nibbles
        for (int j = 0; j < QK4_NIBBLES; j++) {
            float val0 = input_vals[i * QK4_NIBBLES * 2 + j * 2] * inv_delta;
            float val1 = input_vals[i * QK4_NIBBLES * 2 + j * 2 + 1] * inv_delta;

            signed char quant0 = (signed char) fminf(7, fmaxf(-8, roundf(val0)));
            signed char quant1 = (signed char) fminf(7, fmaxf(-8, roundf(val1)));

            // Pack two signed 4-bit values into one signed char
            quantized_nibbles[j] = (quant0 & 0x0F) | (quant1 << 4);
        }

        // Copy packed nibbles to output block
        memcpy(out_block[i].nibbles, quantized_nibbles, sizeof(quantized_nibbles));
    }
}

/**
 * @brief Dequantizes a row of 4-bit quantized values back into float values.
 * @param input_block Pointer to the input quantized block.
 * @param output_vals Pointer to the output array of dequantized floats.
 * @param num_elements Number of elements to be dequantized (must be a multiple of QK4_NIBBLES * 2).
 */
void data_dequantize_row_q4_0(
    const void* restrict input_block, float* restrict output_vals, int num_elements
) {
    assert(num_elements % (QK4_NIBBLES * 2) == 0);

    const block_q4_0* restrict in_block = (const block_q4_0*) input_block;
    int num_blocks = num_elements / (QK4_NIBBLES * 2);

    for (int i = 0; i < num_blocks; i++) {
        float delta = in_block[i].delta;

        for (int j = 0; j < QK4_NIBBLES; j++) {
            signed char quant_pair = in_block[i].nibbles[j];

            // Extract lower nibble (quant0) and apply sign correction if necessary
            signed char quant0
                = (quant_pair & 0x0F) - ((quant_pair & 0x08) ? 16 : 0); // Lower nibble

            // Extract upper nibble (quant1) and apply sign correction if necessary
            signed char quant1 = (quant_pair >> 4) - ((quant_pair & 0x80) ? 16 : 0); // Upper nibble

            output_vals[i * QK4_NIBBLES * 2 + j * 2] = quant0 * delta;
            output_vals[i * QK4_NIBBLES * 2 + j * 2 + 1] = quant1 * delta;
        }
    }
}
