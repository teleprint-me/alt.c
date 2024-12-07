/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/magic.h
 *
 * @brief Provides an interface for managing ALT model files,
 *        including file operations, validation, alignment, and marker handling.
 *
 * This header defines the API for interacting with ALT model files. The primary
 * focus is to partition the file into structured, predictable sections while
 * adhering to the ALT file format specification. Each function is designed to
 * handle a specific aspect of file management without making assumptions about
 * the content of the file.
 */

#ifndef ALT_MAGIC_H
#define ALT_MAGIC_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

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
    FILE* model; /**< File pointer to the open model. */

    /**
     * @brief Opens the model file based on the MagicFile structure.
     */
    MagicState (*open)(struct MagicFile* self);

    /**
     * @brief Validates the model file, checking existence and permissions.
     */
    MagicState (*validate)(struct MagicFile* self);

    /**
     * @brief Closes the model file.
     */
    void (*close)(struct MagicFile* self);
} MagicFile;

// ------------------------- Function Declarations -----------------------------

// ------------------------- MagicFile Management ------------------------------

/**
 * @brief Constructs and initializes a MagicFile instance.
 *
 * @param filepath Path to the file.
 * @param mode Mode in which to open the file (e.g., "rb" for read binary).
 * @return A stack-allocated MagicFile structure.
 *
 * This function initializes a MagicFile instance with no dynamic memory
 * allocation, ensuring fast and efficient operations. Users must open
 * the file before performing any operations.
 */
MagicFile magic_file_create(const char* filepath, const char* mode);

/**
 * @brief Opens the model file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS if the file is successfully opened, MAGIC_FILE_ERROR otherwise.
 *
 * Opens the model file in the specified mode. If the operation fails, the file
 * pointer is set to NULL to ensure predictable behavior.
 */
MagicState magic_file_open(MagicFile* magic_file);

/**
 * @brief Closes the model file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 *
 * Closes the file associated with the MagicFile instance and sets the file
 * pointer to NULL to prevent further operations.
 */
void magic_file_close(MagicFile* magic_file);

/**
 * @brief Validates the Start Marker section of the file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS if valid, MAGIC_ERROR otherwise.
 *
 * Validates the Start Marker section, ensuring the file adheres to the ALT
 * specification. This function checks the header fields (e.g., marker, version,
 * alignment) and resets the file pointer for subsequent operations.
 */
MagicState magic_file_validate(MagicFile* magic_file);

// ----------------------------- Utilities -------------------------------------

/**
 * @brief Guards file operations by checking the MagicFile structure and file pointer.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS if valid, MAGIC_ERROR otherwise.
 *
 * Ensures the file pointer is valid before proceeding with any operation.
 */
MagicState magic_file_guard(MagicFile* magic_file);

/**
 * @brief Aligns the file pointer to the nearest MAGIC_ALIGNMENT boundary.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS if successful, MAGIC_ALIGNMENT_ERROR otherwise.
 *
 * Aligns the file pointer for reading or writing. In read mode, skips alignment
 * bytes. In write mode, adds zero-padding to ensure alignment.
 */
MagicState magic_apply_padding(MagicFile* magic_file);

// ------------------------- Start Marker Functions ----------------------------

/**
 * @brief Writes the Start Marker section.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @param version File format version.
 * @param alignment Alignment requirement for subsequent sections.
 * @return MAGIC_SUCCESS on success, MAGIC_ERROR otherwise.
 *
 * Writes the Start Marker section, including the marker, section size, version,
 * and alignment fields.
 */
MagicState magic_write_start_marker(MagicFile* magic_file, int32_t version, int32_t alignment);

/**
 * @brief Reads and validates the Start Marker section.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @param version Pointer to store the file format version.
 * @param alignment Pointer to store the alignment value.
 * @return MAGIC_SUCCESS on success, MAGIC_ERROR otherwise.
 *
 * Reads the Start Marker section, validates its fields, and outputs the version
 * and alignment values.
 */
MagicState magic_read_start_marker(MagicFile* magic_file, int32_t* version, int32_t* alignment);

// ----------------------- Section Marker Functions ----------------------------

/**
 * @brief Writes a section marker and its size to the model file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @param marker The section marker identifier.
 * @param section_size The size of the section in bytes.
 * @return MAGIC_SUCCESS on success, MAGIC_ERROR otherwise.
 */
MagicState magic_write_section_marker(MagicFile* magic_file, int64_t marker, int64_t section_size);

/**
 * @brief Reads a section marker and its size from the model file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @param marker Pointer to store the section marker identifier.
 * @param section_size Pointer to store the section size in bytes.
 * @return MAGIC_SUCCESS on success, MAGIC_ERROR otherwise.
 */
MagicState magic_read_section_marker(MagicFile* magic_file, int64_t* marker, int64_t* section_size);

// ------------------------ End Marker Functions -------------------------------

/**
 * @brief Writes the end marker (MAGIC_END) to the model file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS on success, MAGIC_ERROR otherwise.
 *
 * Writes the end marker as the absolute end of the file. No data or padding is
 * allowed after the end marker.
 */
MagicState magic_write_end_marker(MagicFile* magic_file);

/**
 * @brief Reads and validates the end marker (MAGIC_END) from the model file.
 *
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS if valid, MAGIC_ERROR otherwise.
 *
 * Reads the end marker and ensures it matches the expected value, confirming
 * the file's structure is intact.
 */
MagicState magic_read_end_marker(MagicFile* magic_file);

#endif // ALT_MAGIC_H
