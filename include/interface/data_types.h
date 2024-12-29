/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/data_types.h
 *
 * @brief API for numeric data types and conversions.
 *
 * Features:
 * - Single and half-precision floating-point support.
 * - 8-bit and 4-bit quantized integer support.
 * - Minimal dependencies with a consistent, extensible design.
 *
 * Notes:
 * - A modern transformer typically consists of 32 blocks.
 * - Each block contains a single layer with 9 sub-layers.
 * - The number of blocks corresponds to the number of layers.
 */

#ifndef ALT_DATA_TYPES_H
#define ALT_DATA_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

#include <assert.h>
#include <math.h>
#include <stdalign.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

// Common mathematical constants

#ifndef M_PI
    #define M_PI 3.141592653589793f // circumference / diameter
#endif

#ifndef PI
    #define PI M_PI
#endif

#ifndef SQRT_2_PI
    #define SQRT_2_PI 0.7978845608028654f // sqrt(2 / pi)
#endif

#ifndef SQRT_2
    #define SQRT_2 1.4142135623730951f // sqrt(2)
#endif

// Safe type casting macros
#define TYPE_CAST(ptr, type) ((type*) (ptr))
#define TYPE_CAST_SAFE(ptr, type, type_object) \
    ((type_object->size == sizeof(*(type*) (ptr))) ? (type*) (ptr) : NULL)

// Block size definitions for quantization
#define BLOCK_SIZE 32 /**< Standard block size for quantization */
#define Q8_ELEMENTS BLOCK_SIZE /**< Elements in an 8-bit quantized block */
#define Q4_NIBBLES (BLOCK_SIZE / 2) /**< Nibbles in a 4-bit quantized block */

// Union for floating-point bit manipulation
typedef union FloatBits {
    float value; /**< Floating-point value */
    uint32_t bits; /**< Bit-level representation */
} FloatBits;

// Quantization structure
typedef struct QuantBits {
    uint8_t bits; /**< Quantized value with baked residual */
    uint16_t scalar; /**< Scaling factor */
} QuantBits;

// Type aliases for quantization
typedef QuantBits Q8; /**< 8-bit quantization */
typedef QuantBits Q4; /**< 4-bit quantization */
typedef QuantBits Q8Row[Q8_ELEMENTS]; /**< Array of 8-bit quantized values */
typedef QuantBits Q4Row[Q4_NIBBLES]; /**< Array of 4-bit quantized values */

// Supported data types
typedef enum DataTypeId {
    TYPE_FLOAT32, /**< 32-bit floating-point (IEEE-754) */
    TYPE_FLOAT16, /**< 16-bit floating-point (IEEE-754) */
    TYPE_QUANT8, /**< 8-bit quantized integer */
    TYPE_QUANT4, /**< 4-bit quantized integer */
    TYPE_INT32, /**< 32-bit signed integer */
    TYPE_INT16, /**< 16-bit signed integer */
    TYPE_INT8, /**< 8-bit signed integer */
    TYPE_UINT32, /**< 32-bit unsigned integer */
    TYPE_UINT16, /**< 16-bit unsigned integer */
    TYPE_UINT8, /**< 8-bit unsigned integer */
    TYPE_BOOL, /**< Boolean */
    TYPE_CHAR, /**< 1-byte character */
    TYPE_WCHAR, /**< Wide character */
    TYPE_COUNT /**< Total number of types */
} DataTypeId;

// Data type sign
typedef enum DataTypeSign {
    TYPE_NOT_APPLICABLE, /**< Not applicable (e.g., for packed types) */
    TYPE_IS_SIGNED, /**< Signed types */
    TYPE_IS_UNSIGNED /**< Unsigned types */
} DataTypeSign;

// Metadata for data types
typedef struct DataType {
    const char* name; /**< Human-readable name */
    uint32_t alignment; /**< Memory alignment in bytes */
    uint32_t size; /**< Size in bytes */
    DataTypeSign sign; /**< Signed/unsigned status */
    DataTypeId id; /**< Unique identifier */
} DataType;

// Static array of supported types
static const DataType TYPES[TYPE_COUNT] = {
    [TYPE_FLOAT32] = {"float32", _Alignof(float),    sizeof(float),    TYPE_IS_SIGNED,      TYPE_FLOAT32},
    [TYPE_FLOAT16] = {"float16", _Alignof(uint16_t), sizeof(uint16_t), TYPE_IS_UNSIGNED,    TYPE_FLOAT16},
    [TYPE_QUANT8] = {"qint8",   _Alignof(Q8),       sizeof(Q8),       TYPE_NOT_APPLICABLE, TYPE_QUANT8 },
    [TYPE_QUANT4] = {"qint4",   _Alignof(Q4),       sizeof(Q4),       TYPE_NOT_APPLICABLE, TYPE_QUANT4 },
    [TYPE_INT32] = {"int32",   _Alignof(int32_t),  sizeof(int32_t),  TYPE_IS_SIGNED,      TYPE_INT32  },
    [TYPE_INT16] = {"int16",   _Alignof(int16_t),  sizeof(int16_t),  TYPE_IS_SIGNED,      TYPE_INT16  },
    [TYPE_INT8] = {"int8",    _Alignof(int8_t),   sizeof(int8_t),   TYPE_IS_SIGNED,      TYPE_INT8   },
    [TYPE_UINT32] = {"uint32",  _Alignof(uint32_t), sizeof(uint32_t), TYPE_IS_UNSIGNED,    TYPE_UINT32 },
    [TYPE_UINT16] = {"uint16",  _Alignof(uint16_t), sizeof(uint16_t), TYPE_IS_UNSIGNED,    TYPE_UINT16 },
    [TYPE_UINT8] = {"uint8",   _Alignof(uint8_t),  sizeof(uint8_t),  TYPE_IS_UNSIGNED,    TYPE_UINT8  },
    [TYPE_BOOL] = {"bool",    _Alignof(bool),     sizeof(bool),     TYPE_NOT_APPLICABLE, TYPE_BOOL   },
    [TYPE_CHAR] = {"char",    _Alignof(char),     sizeof(char),     TYPE_IS_UNSIGNED,    TYPE_CHAR   },
    [TYPE_WCHAR] = {"wchar",   _Alignof(wchar_t),  sizeof(wchar_t),  TYPE_IS_UNSIGNED,    TYPE_WCHAR  }
};

// Data type management
const DataType* data_type_get(DataTypeId id); /**< Retrieve metadata by type ID */
uint32_t data_type_size(DataTypeId id); /**< Get size of type by ID */
const char* data_type_name(DataTypeId id); /**< Get name of type by ID */

// Scalar conversions

// Floating-point encoding/decoding
uint32_t encode_scalar_fp32(float value); /**< Encode 32-bit float to bits */
float decode_scalar_fp32(uint32_t bits); /**< Decode bits to 32-bit float */

// Half-precision floating-point
uint16_t quantize_scalar_fp16(float value); /**< Quantize 32-bit float to 16-bit */
float dequantize_scalar_fp16(uint16_t bits); /**< Dequantize 16-bit to 32-bit float */

// 8-bit integer quantization
Q8 quantize_scalar_q8(float value); /**< Quantize 32-bit float to 8-bit */
float dequantize_scalar_q8(Q8 q8); /**< Dequantize 8-bit to 32-bit float */

// 4-bit integer quantization
Q4 quantize_scalar_q4(float a, float b); /**< Quantize two floats to 4-bit */
float dequantize_scalar_q4_index(Q4 q4, uint32_t index); /**< Dequantize by index */
void dequantize_scalar_q4_reference(Q4 q4, float* a, float* b); /**< Dequantize to references */

// Vector conversions (1D arrays)

// Half-precision floating-point
void quantize_row_fp16(const float* input, uint16_t* output, uint32_t length, uint32_t step_size);
void dequantize_row_fp16(const uint16_t* input, float* output, uint32_t length, uint32_t step_size);

// 8-bit integer quantization
void quantize_row_q8(const float* input, Q8Row output, uint32_t length, uint32_t step_size);
void dequantize_row_q8(const Q8Row input, float* output, uint32_t length, uint32_t step_size);

// 4-bit integer quantization
void quantize_row_q4(const float* input, Q4Row output, uint32_t length, uint32_t step_size);
void dequantize_row_q4(const Q4Row input, float* output, uint32_t length, uint32_t step_size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ALT_DATA_TYPES_H
