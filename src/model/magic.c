/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/model/magic.c
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

#include "interface/logger.h"

#include "model/magic.h"

/**
 * @brief Constructs and initializes a MagicFile instance.
 */
MagicFile* magic_file_open(const char* filepath, const char* mode) {
    // Create a structure on the stack
    MagicFile* magic_file = (MagicFile*) malloc(sizeof(MagicFile));
    if (!magic_file) {
        LOG_ERROR("%s: Failed to allocate memory to MagicFile.\n", __func__);
        return NULL;
    }

    // Add member variables
    magic_file->filepath = filepath;
    magic_file->mode = mode;
    magic_file->data = fopen(magic_file->filepath, magic_file->mode);
    if (!magic_file->data) {
        LOG_ERROR("%s: Unable to open file %s\n", __func__, magic_file->filepath);
        return NULL;
    }

    // Log results
    LOG_DEBUG("%s: MagicFile opened stream for %s in mode %s.\n", __func__, filepath, mode);
    return magic_file;
}

/**
 * @brief Closes the model file.
 */
MagicState magic_file_close(MagicFile* magic_file) {
    MagicState state = magic_file_guard(magic_file);
    if (MAGIC_SUCCESS != state) {
        return state;
    }

    if (magic_file->data) {
        if (0 != fclose(magic_file->data)) {
            LOG_ERROR("%s: Failed to close file stream.\n", __func__);
            return MAGIC_FILE_ERROR;
        }
    }

    free(magic_file);
    LOG_DEBUG("%s: MagicFile closed stream successfully.\n", __func__);
    return MAGIC_SUCCESS;
}

/**
 * @brief Validates the model file, checking existence, permissions, and the header.
 */
MagicState magic_file_validate(MagicFile* magic_file) {
    MagicState state = magic_file_guard(magic_file);
    if (MAGIC_SUCCESS != state) {
        return state;
    }

    // Read and validate the Start Marker
    int32_t version = 0;
    int32_t alignment = 0;
    state = magic_file_read_start_marker(magic_file, &version, &alignment);
    if (MAGIC_SUCCESS != state) {
        LOG_ERROR("%s: Magic marker validation failed.\n", __func__);
        return state;
    }

    // Reset the file pointer to the start
    if (0 != fseek(magic_file->data, 0, SEEK_SET)) {
        LOG_ERROR("%s: Failed to reset the file pointer after validation.\n", __func__);
        return MAGIC_FILE_ERROR;
    }

    LOG_DEBUG(
        "%s: Model file validated successfully. Version: %d, Alignment: %d.\n",
        __func__,
        version,
        alignment
    );

    return MAGIC_SUCCESS;
}

/**
 * @brief Helper function for guarding write and read operations.
 */
MagicState magic_file_guard(MagicFile* magic_file) {
    if (!magic_file) {
        LOG_ERROR("%s: MagicFile is NULL.\n", __func__);
        return MAGIC_ERROR;
    }
    if (!magic_file->data) {
        LOG_ERROR("%s: MagicFile stream is NULL.\n", __func__);
        return MAGIC_FILE_ERROR;
    }
    return MAGIC_SUCCESS;
}

/**
 * @brief Aligns the file pointer to the nearest MAGIC_ALIGNMENT boundary.
 */
MagicState magic_file_pad(MagicFile* magic_file) {
    MagicState state = magic_file_guard(magic_file);
    if (MAGIC_SUCCESS != state) {
        return state;
    }

    long position = ftell(magic_file->data);
    if (position < 0) {
        LOG_ERROR("%s: Failed to get file offset.\n", __func__);
        return MAGIC_ALIGNMENT_ERROR;
    }

    size_t offset = (MAGIC_ALIGNMENT - (position % MAGIC_ALIGNMENT)) % MAGIC_ALIGNMENT;
    if (0 == strcmp(magic_file->mode, "wb")) {
        // Writing mode: Pad with `0x00` bytes
        if (offset > 0) {
            char padding[MAGIC_ALIGNMENT] = {0};
            if (offset != fwrite(padding, 1, offset, magic_file->data)) {
                LOG_ERROR("%s: Failed to write padding bytes.\n", __func__);
                return MAGIC_ALIGNMENT_ERROR;
            }
            LOG_DEBUG("%s: Wrote %ld padding bytes.\n", __func__, offset);
        }
    } else if (0 == strcmp(magic_file->mode, "rb")) {
        // Reading mode: Skip padding bytes
        if (0 != fseek(magic_file->data, offset, SEEK_CUR)) {
            LOG_ERROR("%s: Failed to skip padding bytes.\n", __func__);
            return MAGIC_ALIGNMENT_ERROR;
        }
        LOG_DEBUG("%s: Skipped %ld padding bytes.\n", __func__, offset);
    } else {
        LOG_ERROR("%s: Invalid file stream mode %s", __func__, magic_file->mode);
        return MAGIC_ALIGNMENT_ERROR;
    }

    return MAGIC_SUCCESS;
}

MagicState magic_file_write_start_marker(MagicFile* magic_file, int32_t version, int32_t alignment) {
    MagicState state = magic_file_guard(magic_file);
    if (MAGIC_SUCCESS != state) {
        return state;
    }

    // Define the fields for the magic header
    int64_t marker = MAGIC_ALT;
    int64_t size = sizeof(int64_t) + sizeof(int64_t) + sizeof(int32_t) + sizeof(int32_t);

    // Write the magic header fields
    if (1 != fwrite(&marker, sizeof(int64_t), 1, magic_file->data)
        || 1 != fwrite(&size, sizeof(int64_t), 1, magic_file->data)
        || 1 != fwrite(&version, sizeof(int32_t), 1, magic_file->data)
        || 1 != fwrite(&alignment, sizeof(int32_t), 1, magic_file->data)) {
        LOG_ERROR("%s: Failed to write magic header.\n", __func__);
        return MAGIC_FILE_ERROR;
    }

    // Write alignment padding to end of header
    magic_file_pad(magic_file);

    LOG_DEBUG(
        "%s: Magic header written successfully. "
        "Marker: 0x%lx, Size: %ld, Version: %d, Alignment: %d.\n",
        __func__,
        marker,
        size,
        version,
        alignment
    );

    return MAGIC_SUCCESS;
}

MagicState magic_file_read_start_marker(MagicFile* magic_file, int32_t* version, int32_t* alignment) {
    MagicState state = magic_file_guard(magic_file);
    if (MAGIC_SUCCESS != state) {
        return state;
    }

    // Read the magic header and its required fields
    int64_t marker = 0;
    int64_t size = 0;
    if (1 != fread(&marker, sizeof(int64_t), 1, magic_file->data)
        || 1 != fread(&size, sizeof(int64_t), 1, magic_file->data)
        || 1 != fread(version, sizeof(int32_t), 1, magic_file->data)
        || 1 != fread(alignment, sizeof(int32_t), 1, magic_file->data)) {
        LOG_ERROR("%s: Failed to read magic header.\n", __func__);
        return MAGIC_FILE_ERROR;
    }

    // Validate the magic marker
    if (MAGIC_ALT != marker) {
        LOG_ERROR(
            "%s: Invalid magic header. Expected 0x%lx, got 0x%lx.\n", __func__, MAGIC_ALT, marker
        );
        return MAGIC_INVALID_MARKER;
    }

    // Validate the section size (24 bytes)
    const int64_t expected = sizeof(int32_t) + sizeof(int32_t);
    if (expected != size) {
        LOG_ERROR(
            "%s: Invalid magic header size. Expected %ld, got %ld.\n", __func__, expected, size
        );
        return MAGIC_ERROR;
    }

    // Read alignment padding to advance file pointer
    magic_file_pad(magic_file);

    LOG_DEBUG(
        "%s: Magic header read successfully. "
        "Marker: 0x%lx, Size: %ld, Version: %d, Alignment: %d.\n",
        __func__,
        marker,
        size,
        *version,
        *alignment
    );

    return MAGIC_SUCCESS;
}

/**
 * @brief Writes a section marker and its size to the model file.
 */
MagicState magic_file_write_section_marker(MagicFile* magic_file, int64_t marker, int64_t size) {
    MagicState state = magic_file_guard(magic_file);
    if (MAGIC_SUCCESS != state) {
        return state;
    }

    if (1 != fwrite(&marker, sizeof(int64_t), 1, magic_file->data)
        || 1 != fwrite(&size, sizeof(int64_t), 1, magic_file->data)) {
        LOG_ERROR("%s: Failed to write magic marker %ld or size %ld.\n", __func__, marker, size);
        return MAGIC_FILE_ERROR;
    }

    LOG_DEBUG("%s: Wrote section marker 0x%lx with size %ld.\n", __func__, marker, size);
    return MAGIC_SUCCESS;
}

/**
 * @brief Reads a section marker and its size from the model file.
 */
MagicState magic_file_read_section_marker(MagicFile* magic_file, int64_t* marker, int64_t* size) {
    MagicState state = magic_file_guard(magic_file);
    if (MAGIC_SUCCESS != state) {
        return state;
    }

    if (1 != fread(marker, sizeof(int64_t), 1, magic_file->data)
        || 1 != fread(size, sizeof(int64_t), 1, magic_file->data)) {
        LOG_ERROR("%s: Failed to read section marker or size.\n", __func__);
        return MAGIC_ERROR;
    }

    LOG_DEBUG("%s: Read section marker 0x%lx with size %ld.\n", __func__, *marker, *size);
    return MAGIC_SUCCESS;
}

/**
 * @brief Writes the end marker (MAGIC_END) to the model file.
 */
MagicState magic_file_write_end_marker(MagicFile* magic_file) {
    MagicState state = magic_file_guard(magic_file);
    if (MAGIC_SUCCESS != state) {
        return state;
    }

    int32_t marker = MAGIC_END;
    if (1 != fwrite(&marker, sizeof(int32_t), 1, magic_file->data)) {
        LOG_ERROR("%s: Failed to write end marker.\n", __func__);
        return MAGIC_FILE_ERROR;
    }

    LOG_DEBUG("%s: Wrote end marker.\n", __func__);
    return MAGIC_SUCCESS;
}

/**
 * @brief Reads and validates the end marker (MAGIC_END) from the model file.
 */
MagicState magic_file_read_end_marker(MagicFile* magic_file) {
    MagicState state = magic_file_guard(magic_file);
    if (MAGIC_SUCCESS != state) {
        return state;
    }

    int32_t marker;
    if (1 != fread(&marker, sizeof(int32_t), 1, magic_file->data)) {
        LOG_ERROR("%s: Failed to read end of file.\n", __func__);
        return MAGIC_FILE_ERROR;
    }
    if (MAGIC_END != marker) {
        LOG_ERROR("%s: Invalid end marker.\n", __func__);
        return MAGIC_ERROR;
    }

    LOG_DEBUG("%s: Read and validated end of file.\n", __func__);
    return MAGIC_SUCCESS;
}

// Handle magic fields

MagicState magic_file_read_bool_field(MagicFile* magic_file, bool* field) {
    if (fread(field, sizeof(bool), 1, magic_file->data) != 1) {
        LOG_ERROR("%s: Failed to read bool field.", __func__);
        return MAGIC_FILE_ERROR;
    }
    return MAGIC_SUCCESS;
}

MagicState magic_file_read_int_field(MagicFile* magic_file, int32_t* field) {
    if (fread(field, sizeof(int32_t), 1, magic_file->data) != 1) {
        LOG_ERROR("%s: Failed to read int32_t field.", __func__);
        return MAGIC_FILE_ERROR;
    }
    return MAGIC_SUCCESS;
}

MagicState magic_file_read_float_field(MagicFile* magic_file, float* field) {
    if (fread(field, sizeof(float), 1, magic_file->data) != 1) {
        LOG_ERROR("%s: Failed to read float field.", __func__);
        return MAGIC_FILE_ERROR;
    }
    return MAGIC_SUCCESS;
}

MagicState magic_file_read_string_field(MagicFile* magic_file, char** field) {
    int32_t length = 0;

    // Read the length of the string
    if (fread(&length, sizeof(int32_t), 1, magic_file->data) != 1) {
        LOG_ERROR("%s: Failed to read string length.", __func__);
        return MAGIC_FILE_ERROR;
    }
    if (length <= 0) {
        LOG_ERROR("%s: Invalid string length: %d.", __func__, length);
        return MAGIC_FILE_ERROR;
    }

    // Allocate memory for the string (length + 1 for null terminator)
    *field = (char*) malloc((length + 1) * sizeof(char));
    if (!*field) {
        LOG_ERROR("%s: Memory allocation failed for string.", __func__);
        return MAGIC_ERROR;
    }

    // Read the string data
    if ((size_t) length != fread(*field, sizeof(char), length, magic_file->data)) {
        LOG_ERROR("%s: Failed to read string data.", __func__);
        free(*field);
        *field = NULL; // Prevent dangling pointers
        return MAGIC_FILE_ERROR;
    }

    // Null-terminate the string
    (*field)[length] = '\0';

    return MAGIC_SUCCESS;
}
