/**
 * @file include/data_types.h
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

// Block size definitions for quantization
#define DATA_QUANT_BLOCK_SIZE 32 /**< Standard block size for quantization */
#define Q8_ELEMENTS DATA_QUANT_BLOCK_SIZE /**< Elements in an 8-bit quantized block */
#define Q4_NIBBLES (DATA_QUANT_BLOCK_SIZE / 2) /**< Nibbles in a 4-bit quantized block */

// Macro to compute the minimum of two values
#define MIN(a, b) ((a) < (b) ? (a) : (b))

// Macro to compute the maximum of two values
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// Macro to ensure a value falls within a specific range [min, max]
#define MINMAX(value, min, max) ((value) < (min) ? (min) : ((value) > (max) ? (max) : (value)))

// Macro to clamp a value between a lower and upper bound
#define CLAMP(value, lower, upper) MINMAX((value), (lower), (upper))

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

/**
 * @brief Union for floating-point bit manipulation.
 */
typedef union {
    float value; /**< Floating-point value */
    unsigned int bits; /**< Raw bit representation */
} FloatBits;

typedef struct {
    float delta;  /**< Scaling factor for quantization */
    float min;    /**< Minimum representable value */
    float max;    /**< Maximum representable value */
    unsigned char scalar; /**< Quantized scalar value */
} Q8;

typedef struct {
    float delta;
    float min;
    float max;
    unsigned char scalar; /**< Packed nibble (4 bits) */
} Q4;

typedef Q8 Q8Row[Q8_ELEMENTS];
typedef Q4 Q4Row[Q4_NIBBLES];

// Scalar Conversions

// Floating-point encoding and decoding
unsigned int encode_float32_to_bits(float value);
float decode_bits_to_float32(unsigned int bits);

// Half-precision floating-point quantization
unsigned short quantize_scalar_fp16(float value);
float dequantize_scalar_fp16(unsigned short f16);

// 8-bit integer quantization
Q8 quantize_scalar_q8(float value);
float dequantize_scalar_q8(Q8 q8);

// 4-bit integer quantization
Q4 quantize_scalar_q4(float value1, float value2);
float dequantize_scalar_q4(Q4 q4, int index);

// Vector Conversions (1D arrays)

// Row-based quantization and dequantization
void quantize_row_fp16(const float* input, unsigned short* output, int count);
void dequantize_row_fp16(const unsigned short* input, float* output, int count);

void quantize_row_q8(const float* input, Q8Row output, int count);
void dequantize_row_q8(const Q8Row input, float* output, int count);

void quantize_row_q4(const float* input, Q4Row output, int count);
void dequantize_row_q4(const Q4Row input, float* output, int count);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ALT_DATA_TYPES_H
