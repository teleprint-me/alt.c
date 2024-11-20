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
 */

#ifndef ALT_DATA_TYPES_H
#define ALT_DATA_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <math.h>

// Fixed-point arithmetic

// @brief Number of fractional bits in the fixed-point representation.
#define FIXED_SIZE        16

// @brief Converts a fixed-point number to an integer.
#define FIXED_TO_INT(x)   ((x) >> FIXED_SIZE)

// @brief Converts an integer to a fixed-point number.
#define INT_TO_FIXED(x)   ((x) << FIXED_SIZE)

// @brief Scale between an integer and a fixed-point value.
// @note Fixed-point scaling factor, equivalent to 2^FIXED_SIZE (65536).
#define FIXED_VAL         (1 << FIXED_SIZE)

// @brief Converts a floating-point number to fixed-point format.
#define FLOAT_TO_FIXED(x) ((signed int) ((x) * FIXED_VAL))

// @brief Converts a fixed-point number to floating-point format.
#define FIXED_TO_FLOAT(x) ((float) (x) / FIXED_VAL)

// Half-precision arithmetic

// Quantize F16

// Mask for extracting the exponent bits of f32
#define F32_EXPONENT_MASK 0xFF000000 /**< Isolates the exponent bits from f32 representation */

// Half-precision floating-point bit masks
#define F16_SIGN_MASK 0x80000000 /**< Extracts the sign bit from f32 representation */
#define F16_EXP_MASK 0x00007C00 /**< Isolates the exponent bits in f16 format */
#define F16_MANTISSA_MASK 0x00000FFF /**< Isolates the mantissa bits in f16 format */

// Scaling factors for handling denormalized values
#define F16_SCALE_TO_INF 0x1.0p+112f  /**< Scales small numbers up to avoid denormalization */
#define F16_SCALE_TO_ZERO 0x1.0p-110f /**< Scales large numbers down to fit in f16 range */

// Smallest positive f32 value that maps to f16
#define F16_SMALLEST_EXPONENT 0x71000000 /**< Clamps f32 exponent to the smallest f16 normalized exponent */

// Bias adjustment for f16 exponent
#define F16_BIAS_ADJUSTMENT 0x07800000 /**< Shifts f32 exponent to align with f16 bias and range */

// Overflow marker for f16 (infinity)
#define F16_INFINITY 0x7E00 /**< Represents infinity in f16 format (11111 exponent, 0 mantissa) */

// Dequantize F16

// Bias adjustment for normalized f32 values
#define F16_EXP_OFFSET (0xE0 << 23) /**< Bias adjustment for converting f16 exponent to f32 */

// Scale factor for normalized f32 values
#define F16_EXP_SCALE 0x1.0p-112f /**< Scale factor for normalized values */

// Mask and bias for denormalized f32 values
#define F16_MAGIC_MASK 0x7E000000 /**< Bias mask for denormalized values */
#define F16_MAGIC_BIAS 0.5f       /**< Bias offset for denormalized values */

// Threshold for denormalized cutoff
#define F16_DENORMALIZED_CUTOFF (1 << 27) /**< Threshold for identifying denormalized values */

// Quantized arithmetic

// @note
// A modern transformer typically consists of 32 blocks.
// A block is a single layer composed of 9 sub-layers.
// F32 and F16 will typically contain 32 blocks.

// Block size definitions for quantization
#define DATA_BLOCK_SIZE 32 /**< Standard block size for quantization */
#define Q8_ELEMENTS DATA_BLOCK_SIZE /**< Elements in an 8-bit quantized block */
#define Q4_NIBBLES (DATA_BLOCK_SIZE / 2) /**< Nibbles in a 4-bit quantized block */

// Interval arithmetic

// Macro to ensure a value falls within a specific range [min, max]
#define MINMAX(value, min, max) fmaxf((min), (fminf((value), (max))))

// Macro to clamp a value between a lower and upper bound
#define CLAMP(value, lower, upper) MINMAX((value), (lower), (upper))

// Supported data types

/**
 * @brief Supported data types for this API.
 */
typedef enum {
    TYPE_FLOAT32, /**< IEEE-754 32-bit floating-point */
    TYPE_FLOAT16, /**< IEEE-754 16-bit floating-point */
    TYPE_QINT8, /**< 8-bit integer quantization */
    TYPE_QINT4, /**< 4-bit packed quantization */
    TYPE_COUNT /**< Total number of supported types */
} DataType;

// Single precision conversions

/**
 * @brief Union for floating-point bit manipulation.
 */
typedef union {
    float value; /**< Floating-point value */
    unsigned int bits; /**< Raw bit representation */
} FloatBits;

// Quantized conversions

typedef struct {
    float scalar;  /**< Scaling factor for quantization */
    unsigned char quant; /**< Quantized scalar value */
} Q8;

typedef Q8 Q4; // /**< Packed nibble (4 bits) */
typedef Q8 Q8Row[Q8_ELEMENTS];
typedef Q4 Q4Row[Q4_NIBBLES];

// Scalar Conversions

// Floating-point encoding and decoding
unsigned int encode_float32_to_bits(float value);
float decode_bits_to_float32(unsigned int bits);

// Half-precision floating-point quantization
unsigned short quantize_scalar_fp16(float value);
float dequantize_scalar_fp16(unsigned short bits);

// 8-bit integer quantization (unpacked)
Q8 quantize_scalar_q8(float value);
float dequantize_scalar_q8(Q8 q8);

// 4-bit integer quantization (packed)
Q4 quantize_scalar_q4(float a, float b);
float dequantize_scalar_q4(Q4 q4, int index);

// Vector Conversions (1D arrays)

// Half-precision floating-point quantization
void quantize_row_fp16(const float* input, unsigned short* output, int count);
void dequantize_row_fp16(const unsigned short* input, float* output, int count);

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
