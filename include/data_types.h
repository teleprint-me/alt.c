/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/data_types.h
 * 
 * @brief API for handling numeric data types and conversions.
 *
 * Focused on:
 * - Single and half-precision floating-point.
 * - 8-bit and 4-bit quantized integers.
 * - Minimal dependencies and consistent design.
 * 
 * @note
 * - A modern transformer typically consists of 32 blocks.
 * - A block is a single layer composed of 9 sub-layers.
 * - The number of layers will typically determine the number of blocks.
 */

#ifndef ALT_DATA_TYPES_H
#define ALT_DATA_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <math.h>
#include <stdint.h>

// Block size definitions for quantization
#define BLOCK_SIZE 32 /**< Standard block size for quantization */
#define Q8_ELEMENTS BLOCK_SIZE /**< Elements in an 8-bit quantized block */
#define Q4_NIBBLES (BLOCK_SIZE / 2) /**< Nibbles in a 4-bit quantized block */

// Macro to clamp a value between a lower and upper bound
#define CLAMP(value, lower, upper) fmaxf((lower), (fminf((value), (upper))))

// Supported data types

/**
 * @brief Supported data types for this API.
 */
typedef enum {
    // Floating-point
    TYPE_DOUBLE,       /**< IEEE-754 64-bit floating-point (double) */
    TYPE_FLOAT,        /**< IEEE-754 32-bit floating-point */
    TYPE_FLOAT16,      /**< IEEE-754 16-bit floating-point */
    TYPE_BFLOAT16,     /**< Brain floating-point 16-bit */
    // Signed integers
    TYPE_INT64,        /**< 64-bit signed integer */
    TYPE_INT32,        /**< 32-bit signed integer */
    TYPE_INT16,        /**< 16-bit signed integer */
    TYPE_INT8,         /**< 8-bit signed integer */
    TYPE_INT4,         /**< 4-bit signed integer */
    // Unsigned integers
    TYPE_UINT64,       /**< 64-bit unsigned integer */
    TYPE_UINT32,       /**< 32-bit unsigned integer */
    TYPE_UINT16,       /**< 16-bit unsigned integer */
    TYPE_UINT8,        /**< 8-bit unsigned integer */
    TYPE_UINT4,        /**< 4-bit unsigned integer */
    // Boolean
    TYPE_BOOL,         /**< Boolean type */
    // Character
    TYPE_CHAR,         /**< 1-byte character */
    TYPE_WCHAR,        /**< Wide character */
    // Complex numbers
    TYPE_COMPLEX_FLOAT, /**< Complex number with float components */
    TYPE_COMPLEX_DOUBLE,/**< Complex number with double components */
    // Custom and others
    TYPE_CUSTOM,       /**< User-defined type */
    TYPE_COUNT         /**< Total number of supported types */
} DataType;

// @brief Returns the size of the data type in bytes
uint32_t data_type_size(DataType type);

// Single precision conversions

/**
 * @brief Union for floating-point bit manipulation.
 */
typedef union {
    double value; /**< Floating-point value */
    uint64_t bits; /**< Raw bit representation */
} DoubleBits;

/**
 * @brief Union for floating-point bit manipulation.
 */
typedef union {
    float value; /**< Floating-point value */
    uint32_t bits; /**< Raw bit representation */
} FloatBits;

// Quantized conversions

typedef struct {
    float scalar;  /**< Scaling factor for quantization */
    float min;
    float max;
    uint8_t quant; /**< Quantized scalar value */
} Q8;

typedef Q8 Q4; // /**< Packed nibble (4 bits) */
typedef Q8 Q8Row[Q8_ELEMENTS];
typedef Q4 Q4Row[Q4_NIBBLES];

// Scalar Conversions

uint64_t encode_scalar_fp64(double value); 
double decode_scalar_fp64(uint64_t bits);

// Floating-point encoding and decoding
uint32_t encode_scalar_fp32(float value);
float decode_scalar_fp32(uint32_t bits);

// Half-precision floating-point quantization
uint16_t quantize_scalar_fp16(float value);
float dequantize_scalar_fp16(uint16_t bits);

// 8-bit integer quantization (unpacked)
Q8 quantize_scalar_q8(float value);
float dequantize_scalar_q8(Q8 q8);

// 4-bit integer quantization (packed)
Q4 quantize_scalar_q4(float a, float b);
float dequantize_scalar_q4(Q4 q4, int index);

// Vector Conversions (1D arrays)

// Half-precision floating-point quantization
void quantize_row_fp16(const float* input, uint16_t* output, int count);
void dequantize_row_fp16(const uint16_t* input, float* output, int count);

// 8-bit integer quantization (unpacked)
void quantize_row_q8(const float* input, Q8Row output, int count);
void dequantize_row_q8(const Q8Row input, float* output, int count);

// 4-bit integer quantization (packed)
void quantize_row_q4(const float* input, Q4Row output, int count);
void dequantize_row_q4(const Q4Row input, float* output, int count);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ALT_DATA_TYPES_H
