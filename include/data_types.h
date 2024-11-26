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
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

// Supported data types

#define TYPE_CAST(ptr, type) ((type*) (ptr))
#define TYPE_CAST_SAFE(ptr, type, type_object) \
    ((type_object->size == sizeof(*(type*) (ptr))) ? (type*) (ptr) : NULL)

// Supported data types (as in your original code)
typedef enum {
    TYPE_FLOAT, /**< IEEE-754 32-bit floating-point */
    TYPE_FLOAT16, /**< IEEE-754 16-bit floating-point */
    TYPE_INT32, /**< 32-bit signed integer */
    TYPE_INT16, /**< 16-bit signed integer */
    TYPE_INT8, /**< 8-bit signed integer */
    TYPE_INT4, /**< 4-bit signed integer */
    TYPE_UINT32, /**< 32-bit unsigned integer */
    TYPE_UINT16, /**< 16-bit unsigned integer */
    TYPE_UINT8, /**< 8-bit unsigned integer */
    TYPE_UINT4, /**< 4-bit unsigned integer */
    TYPE_BOOL, /**< Boolean type */
    TYPE_CHAR, /**< 1-byte character */
    TYPE_WCHAR, /**< Wide character */
    TYPE_COUNT /**< Total number of types */
} DataTypeId;

typedef enum {
    TYPE_NOT_APPLICABLE,
    TYPE_IS_SIGNED,
    TYPE_IS_UNSIGNED
} DataTypeSign;

typedef struct {
    const char* name; /**< Human-readable name of the type */
    uint32_t alignment; /**< Memory alignment in bytes */
    uint32_t size; /**< Size in bytes */
    DataTypeSign sign; /**< Signed, unsigned, or not applicable */
    DataTypeId id; /**< Unique type ID */
} DataType;

// Static array of types
static const DataType TYPES[TYPE_COUNT] = {
    [TYPE_FLOAT] = {"float32", _Alignof(float),    sizeof(float),    TYPE_IS_SIGNED,      TYPE_FLOAT  },
    [TYPE_FLOAT16] = {"float16", _Alignof(uint16_t), sizeof(uint16_t), TYPE_IS_UNSIGNED,    TYPE_FLOAT16},
    [TYPE_INT32] = {"int32",   _Alignof(int32_t),  sizeof(int32_t),  TYPE_IS_SIGNED,      TYPE_INT32  },
    [TYPE_INT16] = {"int16",   _Alignof(int16_t),  sizeof(int16_t),  TYPE_IS_SIGNED,      TYPE_INT16  },
    [TYPE_INT8] = {"int8",    _Alignof(int8_t),   sizeof(int8_t),   TYPE_IS_SIGNED,      TYPE_INT8   },
    [TYPE_INT4] = {"int4",    _Alignof(int8_t),   sizeof(int8_t),   TYPE_IS_SIGNED,      TYPE_INT4   }, // Packed
    [TYPE_UINT32] = {"uint32",  _Alignof(uint32_t), sizeof(uint32_t), TYPE_IS_UNSIGNED,    TYPE_UINT32 },
    [TYPE_UINT16] = {"uint16",  _Alignof(uint16_t), sizeof(uint16_t), TYPE_IS_UNSIGNED,    TYPE_UINT16 },
    [TYPE_UINT8] = {"uint8",   _Alignof(uint8_t),  sizeof(uint8_t),  TYPE_IS_UNSIGNED,    TYPE_UINT8  },
    [TYPE_UINT4] = {"uint4",   _Alignof(uint8_t),  sizeof(uint8_t),  TYPE_IS_UNSIGNED,    TYPE_UINT4  }, // Packed
    [TYPE_BOOL] = {"bool",    _Alignof(bool),     sizeof(bool),     TYPE_NOT_APPLICABLE, TYPE_BOOL   },
    [TYPE_CHAR] = {"char",    _Alignof(char),     sizeof(char),     TYPE_IS_UNSIGNED,    TYPE_CHAR   },
    [TYPE_WCHAR] = {"wchar",   _Alignof(wchar_t),  sizeof(wchar_t),  TYPE_IS_UNSIGNED,    TYPE_WCHAR  }
};

// Data type management
const DataType* data_type_get(DataTypeId id);
uint32_t data_type_size(DataTypeId id);
const char* data_type_name(DataTypeId id);

// Quantized conversions

// Block size definitions for quantization
#define BLOCK_SIZE 32 /**< Standard block size for quantization */
#define Q8_ELEMENTS BLOCK_SIZE /**< Elements in an 8-bit quantized block */
#define Q4_NIBBLES (BLOCK_SIZE / 2) /**< Nibbles in a 4-bit quantized block */

// @brief Union for floating-point bit manipulation.
typedef union {
    float value; /**< Floating-point value */
    uint32_t bits; /**< Raw bit representation */
} FloatBits;

typedef struct QuantMetaData {
    uint8_t bits;
    float alpha;
    float step_size;
    float residual;
} QuantMetaData;

// Quantization structure
typedef struct QuantBits {
    uint8_t bits; /**< Quantized value with baked residual */
    uint16_t scalar; /**< Scaling factor for quantization */
} QuantBits;

typedef QuantBits Q8;
typedef QuantBits Q4; // /**< Packed nibble (4 bits) */
typedef QuantBits Q8Row[Q8_ELEMENTS];
typedef QuantBits Q4Row[Q4_NIBBLES];

// Scalar Conversions

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
// By-Value API: Use for single-value dequantization.
float dequantize_scalar_q4_index(Q4 q4, uint32_t index);
// By-Reference API: Use for batch processing or when both values are typically needed.
void dequantize_scalar_q4_reference(Q4 q4, float* a, float* b);

// Vector Conversions (1D arrays)

// Half-precision floating-point quantization
void quantize_row_fp16(const float* input, uint16_t* output, uint32_t length, uint32_t step_size);
void dequantize_row_fp16(const uint16_t* input, float* output, uint32_t length, uint32_t step_size);

// 8-bit integer quantization (unpacked)
void quantize_row_q8(const float* input, Q8Row output, uint32_t length, uint32_t step_size);
void dequantize_row_q8(const Q8Row input, float* output, uint32_t length, uint32_t step_size);

// 4-bit integer quantization (packed)
void quantize_row_q4(const float* input, Q4Row output, uint32_t length, uint32_t step_size);
void dequantize_row_q4(const Q4Row input, float* output, uint32_t length, uint32_t step_size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ALT_DATA_TYPES_H
