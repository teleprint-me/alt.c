/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/magic.h
 *
 * @brief Provides an interface for managing ALT model files,
 *        including file operations, validation, and marker handling.
 */

#ifndef ALT_MAGIC_H
#define ALT_MAGIC_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Constants denoting partitioned sectors for model data.
#define MAGIC_ALT 0x616C7400 // 8 bytes; 'alt' in hex
#define MAGIC_GENERAL 0xCAFEBABE // 8 bytes
#define MAGIC_PARAMETERS 0xDEADBEEF // 8 bytes
#define MAGIC_TOKENIZER 0xBADDCAFE // 8 bytes
#define MAGIC_TENSORS 0xFACEFEED // 8 bytes
#define MAGIC_END 0x0FFFFFFF // 8 bytes
#define MAGIC_ALIGNMENT 32 // Default ALT alignment value
#define MAGIC_VERSION 3 // ALT model file format

// Constants denoting state change for model data.
typedef enum MagicState {
    MAGIC_SUCCESS,
    MAGIC_ERROR,
    MAGIC_INVALID_MARKER,
    MAGIC_ALIGNMENT_ERROR,
    MAGIC_FILE_ERROR
} MagicState;

/**
 * @struct MagicFile
 * @brief Encapsulates ALT model file metadata and operations.
 */
typedef struct MagicFile {
    const char* filepath; /**< Path to the model file */
    const char* mode; /**< File mode (e.g., "rb" for read binary) */
    FILE* model; /**< File pointer to the open model */

    // Member function pointers
    MagicState (*open)(struct MagicFile* self);
    MagicState (*validate)(struct MagicFile* self);
    void (*close)(struct MagicFile* self);
} MagicFile;

// Method declarations for MagicFile

/**
 * @brief Opens the model file based on the MagicFile structure.
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS if successful, MAGIC_FILE_ERROR on failure.
 */
MagicState magic_file_open(MagicFile* magic_file);

/**
 * @brief Validates the model file, checking existence and permissions.
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS if valid, MAGIC_ERROR if invalid.
 */
MagicState magic_file_validate(MagicFile* magic_file);

/**
 * @brief Closes the model file.
 * @param magic_file Pointer to the MagicFile structure.
 */
void magic_file_close(MagicFile* magic_file);

// MagicFile constructor (no destructor is needed as no memory is allocated)

/**
 * @brief Constructs and initializes a MagicFile instance.
 * @param filepath Path to the file.
 * @param mode Mode in which to open the file.
 * @return Initialized MagicFile structure.
 */
MagicFile magic_file_create(const char* filepath, const char* mode);

// Utilities

/**
 * @brief Helper function for guarding write and read operations.
 *        Ensures the MagicFile structure and file pointer are valid.
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS if valid, MAGIC_ERROR if invalid.
 */
MagicState magic_file_guard(MagicFile* magic_file);

// Alignment

/**
 * @brief Aligns the file pointer to the nearest MAGIC_ALIGNMENT boundary.
 * @param magic_file Pointer to the MagicFile structure.
 */
void magic_apply_padding(MagicFile* magic_file);

// Section handling functions

/**
 * @brief Writes a section marker and its size to the model file.
 * @param magic_file Pointer to the MagicFile structure.
 * @param marker The section marker identifier.
 * @param section_size The size of the section in bytes.
 * @return MAGIC_SUCCESS on success, MAGIC_ERROR on failure.
 */
MagicState magic_write_section_marker(MagicFile* magic_file, int64_t marker, int64_t section_size);

/**
 * @brief Reads a section marker and its size from the model file.
 * @param magic_file Pointer to the MagicFile structure.
 * @param marker Pointer to store the section marker identifier.
 * @param section_size Pointer to store the section size in bytes.
 * @return MAGIC_SUCCESS on success, MAGIC_ERROR on failure.
 */
MagicState magic_read_section_marker(MagicFile* magic_file, int64_t* marker, int64_t* section_size);

// Marker handling functions

/**
 * @brief Writes the end marker (MAGIC_END) to the model file.
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS on success, MAGIC_ERROR on failure.
 */
MagicState magic_write_end_marker(MagicFile* magic_file);

/**
 * @brief Reads and validates the end marker (MAGIC_END) from the model file.
 * @param magic_file Pointer to the MagicFile structure.
 * @return MAGIC_SUCCESS if valid, MAGIC_ERROR if invalid.
 */
MagicState magic_read_end_marker(MagicFile* magic_file);

#endif // ALT_MAGIC_H
