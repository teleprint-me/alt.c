/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/model/magic.h
 * @brief Provides an interface for managing ALT model files,
 *        including file operations, validation, alignment, and marker handling.
 *
 * This header defines the API for interacting with ALT model files. The primary
 * focus is to partition the file into structured, predictable sections while
 * adhering to the ALT file format specification. Each function is designed to
 * handle a specific aspect of file management without making assumptions about
 * the content of the file.
 */

#ifndef ALT_MODEL_MAGIC_H
#define ALT_MODEL_MAGIC_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>

// --------------------------------- Constants ---------------------------------

/**
 * @brief Constants representing unique section markers for model data.
 *
 * Each marker identifies a specific section within the ALT model file. These
 * values are written as 64-bit integers to define sections unambiguously.
 */
#define MAGIC_ALT 0x616C7400 /**< File identifier ("alt" in hex). */
#define MAGIC_GENERAL 0xCAFEBABE /**< General metadata section. */
#define MAGIC_PARAMETERS 0xDEADBEEF /**< Model parameters section. */
#define MAGIC_TOKENIZER 0xBADDCAFE /**< Tokenizer data section. */
#define MAGIC_TENSORS 0xFACEFEED /**< Tensor data section. */
#define MAGIC_END 0x0FFFFFFF /**< End marker (absolute end of the file). */
#define MAGIC_ALIGNMENT 32 /**< Default alignment (32 bytes). */
#define MAGIC_VERSION 2 /**< Current ALT file format version. */

// --------------------------- MagicState Enum ---------------------------------

/**
 * @brief Enumeration of possible states for file operations.
 */
typedef enum MagicState {
    MAGIC_SUCCESS, /**< Operation succeeded. */
    MAGIC_ERROR, /**< General error during the operation. */
    MAGIC_INVALID_MARKER, /**< Invalid section marker encountered. */
    MAGIC_ALIGNMENT_ERROR, /**< Alignment error during reading/writing. */
    MAGIC_FILE_ERROR /**< File operation error (e.g., open/close failure). */
} MagicState;

// ---------------------------- MagicFile Struct --------------------------------

/**
 * @struct MagicFile
 * @brief Encapsulates ALT model file metadata and operations.
 *
 * This struct represents an open ALT model file and provides function pointers
 * for opening, validating, and closing the file. It ensures lightweight and
 * predictable operations without unnecessary assumptions.
 */
typedef struct MagicFile {
    const char* filepath; /**< Path to the model file. */
    const char* mode; /**< File mode (e.g., "rb" for read binary). */
    FILE* data; /**< File pointer to the open model. */
} MagicFile;

// ------------------------- Function Declarations -----------------------------

// ------------------------- MagicFile Management ------------------------------

/**
 * @brief Opens the model file.
 *
 * @param filepath The path to the model file to be opened.
 * @param mode The mode in which to open the file (e.g., "rb" for read binary).
 *
 * Opens the model file in the specified mode, creating a MagicFile instance if successful.
 * Returns a MagicFile pointer on success, or NULL on failure.
 *
 * @return A MagicFile pointer on success, or NULL on failure.
 */
MagicFile* magic_file_open(const char* filepath, const char* mode);

/**
 * @brief Closes the model file.
 *
 * @param magic_file A pointer to the MagicFile instance to be closed.
 *
 * Closes the file associated with the MagicFile instance, freeing any allocated resources.
 * Returns MAGIC_SUCCESS if successful, or MAGIC_FILE_ERROR if the file cannot be closed.
 *
 * @return MAGIC_SUCCESS on success, or MAGIC_FILE_ERROR on failure.
 */
MagicState magic_file_close(MagicFile* magic_file);

// ----------------------------- Utilities -------------------------------------

/**
 * @brief Validates the magic header of the file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 *
 * Validates the magic header, ensuring the file adheres to the ALT
 * specification. This function checks the header fields and resets the file
 * pointer for subsequent operations.
 *
 * @return MAGIC_SUCCESS if valid, MAGIC_FILE_ERROR otherwise.
 */
MagicState magic_file_validate(MagicFile* magic_file);

/**
 * @brief Guards file operations by checking the MagicFile structure and file pointer.
 *
 * @param magic_file Pointer to the MagicFile structure.
 *
 * Ensures the file pointer is valid before proceeding with any operation.
 *
 * @return MAGIC_SUCCESS if valid, MAGIC_ERROR otherwise.
 */
MagicState magic_file_guard(MagicFile* magic_file);

/**
 * @brief Aligns the file pointer to the nearest MAGIC_ALIGNMENT boundary.
 *
 * @param magic_file Pointer to the MagicFile structure.
 *
 * Aligns the file pointer for reading or writing. In read mode, skips alignment
 * bytes. In write mode, adds zero-padding to ensure alignment.
 *
 * @return MAGIC_SUCCESS if successful, MAGIC_ALIGNMENT_ERROR otherwise.
 */
MagicState magic_file_pad(MagicFile* magic_file);

// ------------------------- Start Marker Functions ----------------------------

/**
 * @brief Writes the Start Marker section of the file.
 *
 * Writes the magic header fields, including the marker, section size, version,
 * and alignment, ensuring the file adheres to the ALT specification.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @param version File format version.
 * @param alignment Alignment requirement for subsequent sections.
 *
 * @return MAGIC_SUCCESS on success, MAGIC_FILE_ERROR otherwise.
 */
MagicState magic_file_write_start_marker(MagicFile* magic_file, int32_t version, int32_t alignment);

/**
 * @brief Reads the Start Marker section of the file.
 *
 * Reads the magic header fields, including the marker, section size, version,
 * and alignment, ensuring the file adheres to the ALT specification.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @param version Pointer to an int32_t to store the file format version.
 * @param alignment Pointer to an int32_t to store the alignment requirement for subsequent sections.
 *
 * @return MAGIC_SUCCESS on success, MAGIC_FILE_ERROR otherwise.
 */
MagicState magic_file_read_start_marker(MagicFile* magic_file, int32_t* version, int32_t* alignment);

// ----------------------- Section Marker Functions ----------------------------

/**
 * @brief Writes a section marker and its size to the model file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @param marker The section marker identifier.
 * @param size The size of the section in bytes.
 *
 * Writes a section marker and its size to the model file.
 *
 * @return MAGIC_SUCCESS on success, MAGIC_FILE_ERROR otherwise.
 */
MagicState magic_file_write_section_marker(MagicFile* magic_file, int64_t marker, int64_t size);

/**
 * @brief Reads a section marker and its size from the model file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @param marker Pointer to store the section marker identifier.
 * @param size Pointer to store the section size in bytes.
 *
 * Reads a section marker and its size from the model file.
 *
 * @return MAGIC_SUCCESS on success, MAGIC_FILE_ERROR otherwise.
 */
MagicState magic_file_read_section_marker(MagicFile* magic_file, int64_t* marker, int64_t* size);

// ------------------------ End Marker Functions -------------------------------

/**
 * @brief Writes the end marker (MAGIC_END) to the model file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 *
 * Writes the end marker as the absolute end of the file. No data or padding is
 * allowed after the end marker.
 *
 * @return MAGIC_SUCCESS on success, MAGIC_FILE_ERROR otherwise.
 */
MagicState magic_file_write_end_marker(MagicFile* magic_file);

/**
 * @brief Reads and validates the end marker (MAGIC_END) from the model file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 *
 * Reads the end marker and ensures it matches the expected value, confirming
 * the file's structure is intact.
 *
 * @return MAGIC_SUCCESS if valid, MAGIC_FILE_ERROR otherwise.
 */
MagicState magic_file_read_end_marker(MagicFile* magic_file);

// ------------------------ Magic Field Functions ------------------------

MagicState magic_file_read_bool_field(MagicFile* magic_file, bool* field);
MagicState magic_file_read_int_field(MagicFile* magic_file, int32_t* field);
MagicState magic_file_read_float_field(MagicFile* magic_file, float* field);
MagicState magic_file_read_string_field(MagicFile* magic_file, char** field);

// ------------------------ Magic Macro Functions ------------------------

/// @note These macros must be used within a function call.

#define MAGIC_READ_FIELD(magic_file, struct_ptr, field, read_func, field_type, label, free_callback) \
    if (MAGIC_SUCCESS != read_func(magic_file, &struct_ptr->field)) { \
        LOG_ERROR("%s: Failed to read field '%s' in section '%s'.\n", __func__, #field, label); \
        free_callback(struct_ptr); \
        return NULL; \
    }

#define MAGIC_READ_BOOL(magic_file, struct_ptr, bool_field, label, free_callback) \
    MAGIC_READ_FIELD(magic_file, struct_ptr, bool_field, magic_file_read_bool_field, bool, label, free_callback)

#define MAGIC_READ_INT32(magic_file, struct_ptr, int32_field, label, free_callback) \
    MAGIC_READ_FIELD(magic_file, struct_ptr, int32_field, magic_file_read_int_field, int32_t, label, free_callback)

#define MAGIC_READ_FLOAT(magic_file, struct_ptr, float_field, label, free_callback) \
    MAGIC_READ_FIELD(magic_file, struct_ptr, float_field, magic_file_read_float_field, float, label, free_callback)

#define MAGIC_READ_STRING(magic_file, struct_ptr, string_field, label, free_callback) \
    MAGIC_READ_FIELD(magic_file, struct_ptr, string_field, magic_file_read_string_field, char*, label, free_callback)

#endif // ALT_MODEL_MAGIC_H
