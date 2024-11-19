/**
 * @file include/data_types.h
 *
 * @brief API for handling numeric data types and conversions, focusing on 32-bit
 *        floating-point (float) and 32-bit integer (int32_t) representations. 
 *        Future extensions may include 16-bit and 8-bit formats for digital 
 *        signal processing (DSP) applications.
 *
 * This implementation uses pure C with minimal dependencies on external libraries.
 *
 * Key considerations:
 * - The interface is kept minimal and focused.
 * - Avoids the use of generics, limiting to a single base type (float, int32_t).
 * - Conversion logic is isolated into utility functions.
 * - A clean separation is maintained between different components for clarity.
 */

#ifndef ALT_DATA_TYPES_H
#define ALT_DATA_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// Block size definitions for quantization
#define DATA_QUANT_BLOCK_SIZE 32    /**< Standard block size for quantization */
#define QK8_ELEMENTS DATA_QUANT_BLOCK_SIZE  /**< Number of elements in a q8 block */
#define QK4_NIBBLES (DATA_QUANT_BLOCK_SIZE / 2) /**< Number of nibbles in a q4 block */

/**
 * @brief Enumeration for different data types used in this API.
 *
 * @param TYPE_FLOAT32 IEEE-754 32-bit floating-point precision.
 * @param TYPE_FLOAT16 IEEE-754 16-bit floating-point precision.
 * @param TYPE_QINT8 8-bit integer precision (8 elements per block).
 * @param TYPE_QINT4 Packed 8-bit integer precision (4 elements per block, 2 bits per element).
 * @param TYPE_TYPES Total number of data types supported.
 */
typedef enum DataType {
    TYPE_FLOAT32, /**< IEEE-754 32-bit floating-point precision */
    TYPE_FLOAT16, /**< IEEE-754 16-bit floating-point precision */
    TYPE_QINT8,   /**< 8-bit integer precision (8 elements per block) */
    TYPE_QINT4,   /**< Packed 8-bit integer precision (4 elements per block, 2 bits per element) */
    TYPE_TYPES    /**< Number of data types supported */
} DataType;

/**
 * @brief Union representing a flexible floating-point representation.
 *
 * This union allows access to both the raw bit representation of a floating-point
 * number and its actual 32-bit value.
 *
 * @param value 32-bit floating-point value.
 * @param bits Raw 32-bit integer bit representation of the floating-point number.
 */
typedef union UnionType {
    float value;  /**< The actual floating-point value */
    unsigned int bits; /**< Raw 32-bit integer bit representation */
} UnionType;

/**
 * @brief Structure representing an 8-bit quantization block with a scaling factor.
 *
 * This structure holds a block of 8-bit quantized values, along with a scaling factor
 * (delta) to adjust the range of quantized values.
 */
typedef struct {
    float delta; /**< Scaling factor for the quantized values */
    signed char elements[QK8_ELEMENTS]; /**< 8-bit quantized values (one element per byte) */
} block_q8_0;

/**
 * @brief Structure representing a 4-bit quantization block with a scaling factor.
 *
 * This structure holds a block of 4-bit quantized values stored as nibbles, along with
 * a scaling factor (delta) to adjust the range of quantized values.
 */
typedef struct {
    float delta; /**< Scaling factor for the quantized values */
    signed char nibbles[QK4_NIBBLES]; /**< 4-bit quantized values stored as nibbles (half-byte) */
} block_q4_0;

/**
 * @brief Encodes a given floating-point value into its corresponding 32-bit integer
 *        representation (IEEE-754 format).
 *
 * @param[in] value The floating-point value to encode.
 *
 * @return The resulting encoded 32-bit integer representation of the input value.
 */
unsigned int data_encode_float32(float value);

/**
 * @brief Decodes a 32-bit integer representation into its corresponding floating-point value.
 *
 * @param[in] bits The encoded 32-bit integer bit representation of the floating-point number.
 *
 * @return The decoded 32-bit floating-point value.
 */
float data_decode_float32(unsigned int bits);

/**
 * @brief Converts a 16-bit floating-point (half precision) value to a 32-bit floating-point value.
 *
 * @param[in] fp16_val The 16-bit floating-point value to convert.
 *
 * @return The converted 32-bit floating-point value.
 */
float data_fp16_to_fp32(unsigned short fp16_val);

/**
 * @brief Converts a 32-bit floating-point value to a 16-bit floating-point (half precision) value.
 *
 * @param[in] fp32_val The 32-bit floating-point value to convert.
 *
 * @return The converted 16-bit floating-point value.
 */
unsigned short data_fp32_to_fp16(float fp32_val);

/**
 * @brief Quantizes a row of floating-point values into an 8-bit quantization block (q8_0).
 *
 * @param[in] input_vals Pointer to the array of input floating-point values to quantize.
 * @param[out] output_block Pointer to the output block where quantized values will be stored.
 * @param[in] num_elements The number of elements to quantize.
 */
void data_quantize_row_q8_0(
    const float* restrict input_vals, void* restrict output_block, int num_elements
);

/**
 * @brief Dequantizes an 8-bit quantization block (q8_0) back into floating-point values.
 *
 * @param[in] input_block Pointer to the input quantization block to dequantize.
 * @param[out] output_vals Pointer to the output array where dequantized values will be stored.
 * @param[in] num_elements The number of elements to dequantize.
 */
void data_dequantize_row_q8_0(
    const void* restrict input_block, float* restrict output_vals, int num_elements
);

/**
 * @brief Quantizes a row of floating-point values into a 4-bit quantization block (q4_0).
 *
 * @param[in] input_vals Pointer to the array of input floating-point values to quantize.
 * @param[out] output_block Pointer to the output block where quantized values will be stored.
 * @param[in] num_elements The number of elements to quantize.
 */
void data_quantize_row_q4_0(
    const float* restrict input_vals, void* restrict output_block, int num_elements
);

/**
 * @brief Dequantizes a 4-bit quantization block (q4_0) back into floating-point values.
 *
 * @param[in] input_block Pointer to the input quantization block to dequantize.
 * @param[out] output_vals Pointer to the output array where dequantized values will be stored.
 * @param[in] num_elements The number of elements to dequantize.
 */
void data_dequantize_row_q4_0(
    const void* restrict input_block, float* restrict output_vals, int num_elements
);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ALT_DATA_TYPES_H
