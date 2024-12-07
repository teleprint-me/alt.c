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

// Constants denoting partitioned sectors for model data.
#define MAGIC_ALT 0x616C7463 // 4 bytes
#define MAGIC_PARAMETERS 0xDEADBEEF // 8 bytes
#define MAGIC_TOKENIZER 0xBADDCAFE // 8 bytes
#define MAGIC_TENSOR 0xFACEFEED // 8 bytes
#define MAGIC_END 0x0FFFFFFF // 4 bytes
#define MAGIC_ALIGNMENT 32 // Default ALT alignment value

// Constants denoting state change for model data.
#define MAGIC_SUCCESS 0
#define MAGIC_ERROR 1

/**
 * @struct MagicFile
 * @brief Encapsulates ALT model file metadata and operations.
 */
typedef struct MagicFile {
    const char* filepath; /**< Path to the model file */
    const char* mode; /**< File mode (e.g., "rb" for read binary) */
    FILE* model; /**< File pointer to the open model */

    // Member function pointers
    FILE* (*open)(struct MagicFile* self);
    int (*validate)(struct MagicFile* self);
    void (*close)(struct MagicFile* self);
} MagicFile;

// Method declarations for MagicFile

/**
 * @brief Opens the model file based on the MagicFile structure.
 * @param magic_file Pointer to the MagicFile structure.
 * @return FILE* pointer if successful, NULL on failure.
 */
FILE* magic_file_open(MagicFile* magic_file);

/**
 * @brief Validates the model file, checking existence and permissions.
 * @param magic_file Pointer to the MagicFile structure.
 * @return 1 if valid, 0 if invalid.
 */
int magic_file_validate(MagicFile* magic_file);

/**
 * @brief Closes the model file.
 * @param magic_file Pointer to the MagicFile structure.
 */
void magic_file_close(MagicFile* magic_file);

/**
 * @brief Constructs and initializes a MagicFile instance.
 * @param filepath Path to the file.
 * @param mode Mode in which to open the file.
 * @return Initialized MagicFile structure.
 */
MagicFile magic_file_create(const char* filepath, const char* mode);

/**
 * @brief Helper function for guarding write and read operations.
 *        Ensures the MagicFile structure and file pointer are valid.
 * @param magic_file Pointer to the MagicFile structure.
 * @return 1 if valid, 0 if invalid.
 */
int magic_file_guard(MagicFile* magic_file);

// Write functions

/**
 * @brief Writes the start marker (MAGIC_GGML) to the model file.
 * @param magic_file Pointer to the MagicFile structure.
 * @return 0 on success, -1 on failure.
 */
int magic_write_start_marker(MagicFile* magic_file);

/**
 * @brief Writes the end marker (MAGIC_END) to the model file.
 * @param magic_file Pointer to the MagicFile structure.
 * @return 0 on success, -1 on failure.
 */
int magic_write_end_marker(MagicFile* magic_file);

/**
 * @brief Writes a section marker and its size to the model file.
 * @param magic_file Pointer to the MagicFile structure.
 * @param marker The section marker identifier.
 * @param section_size The size of the section in bytes.
 * @return 0 on success, -1 on failure.
 */
int magic_write_section_marker(MagicFile* magic_file, int64_t marker, int64_t section_size);

// Read functions

/**
 * @brief Reads and validates the start marker (MAGIC_GGML) from the model file.
 * @param magic_file Pointer to the MagicFile structure.
 * @return 0 if valid, -1 if invalid.
 */
int magic_read_start_marker(MagicFile* magic_file);

/**
 * @brief Reads and validates the end marker (MAGIC_END) from the model file.
 * @param magic_file Pointer to the MagicFile structure.
 * @return 0 if valid, -1 if invalid.
 */
int magic_read_end_marker(MagicFile* magic_file);

/**
 * @brief Reads a section marker and its size from the model file.
 * @param magic_file Pointer to the MagicFile structure.
 * @param marker Pointer to store the section marker identifier.
 * @param section_size Pointer to store the section size in bytes.
 * @return 0 on success, -1 on failure.
 */
int magic_read_section_marker(MagicFile* magic_file, int64_t* marker, int64_t* section_size);

#endif // ALT_MAGIC_H
