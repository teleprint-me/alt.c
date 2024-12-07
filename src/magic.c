/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/magic.c
 *
 * @brief Provides an implementation for managing ALT model files,
 *        including file operations, validation, and marker handling.
 */

#include "magic.h"
#include "logger.h"

/**
 * @brief Opens the model file based on the MagicFile structure.
 */
MagicState magic_file_open(MagicFile* magic_file) {
    if (magic_file == NULL) {
        return MAGIC_ERROR;
    }
    magic_file->model = fopen(magic_file->filepath, magic_file->mode);
    if (magic_file->model == NULL) {
        LOG_ERROR("%s: Unable to open file %s\n", __func__, magic_file->filepath);
        return MAGIC_FILE_ERROR;
    }
    return MAGIC_SUCCESS;
}

/**
 * @brief Validates the model file, checking existence, permissions, and the header.
 */
MagicState magic_file_validate(MagicFile* magic_file) {
    if (magic_file == NULL || magic_file->model == NULL) {
        LOG_ERROR("%s: Invalid file pointer.\n", __func__);
        return MAGIC_ERROR;
    }

    if (access(magic_file->filepath, F_OK | (magic_file->mode[0] == 'r' ? R_OK : W_OK)) != 0) {
        LOG_ERROR("%s: File %s is inaccessible or does not meet the required permissions.\n", __func__, magic_file->filepath);
        return MAGIC_FILE_ERROR;
    }

    // Validate the start marker, version, and alignment
    int32_t start_marker, version, alignment;
    if (fread(&start_marker, sizeof(int32_t), 1, magic_file->model) != 1 ||
        fread(&version, sizeof(int32_t), 1, magic_file->model) != 1 ||
        fread(&alignment, sizeof(int32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read header from file %s.\n", __func__, magic_file->filepath);
        return MAGIC_ERROR;
    }

    if (start_marker != MAGIC_ALT) {
        LOG_ERROR("%s: Invalid start marker in file %s. Expected 0x%x, got 0x%x.\n", __func__, magic_file->filepath, MAGIC_ALT, start_marker);
        return MAGIC_ERROR;
    }

    if (version != MAGIC_VERSION) {
        LOG_ERROR("%s: Unsupported version in file %s. Expected %d, got %d.\n", __func__, magic_file->filepath, MAGIC_VERSION, version);
        return MAGIC_ERROR;
    }

    if (alignment != MAGIC_ALIGNMENT) {
        LOG_ERROR("%s: Invalid alignment in file %s. Expected %d, got %d.\n", __func__, magic_file->filepath, MAGIC_ALIGNMENT, alignment);
        return MAGIC_ERROR;
    }

    // Reset the file pointer for subsequent operations
    fseek(magic_file->model, 0, SEEK_SET);

    return MAGIC_SUCCESS;
}

/**
 * @brief Closes the model file.
 */
void magic_file_close(MagicFile* magic_file) {
    if (magic_file && magic_file->model) {
        fclose(magic_file->model);
        magic_file->model = NULL;
    }
}

/**
 * @brief Constructs and initializes a MagicFile instance.
 */
MagicFile magic_file_create(const char* filepath, const char* mode) {
    MagicFile magic_file;
    magic_file.filepath = filepath;
    magic_file.mode = mode;
    magic_file.model = NULL;
    magic_file.open = magic_file_open;
    magic_file.validate = magic_file_validate;
    magic_file.close = magic_file_close;
    return magic_file;
}

/**
 * @brief Helper function for guarding write and read operations.
 */
MagicState magic_file_guard(MagicFile* magic_file) {
    if (magic_file == NULL || magic_file->model == NULL) {
        LOG_ERROR("%s: Invalid file pointer.\n", __func__);
        return MAGIC_ERROR;
    }
    return MAGIC_SUCCESS;
}

/**
 * @brief Aligns the file pointer to the nearest MAGIC_ALIGNMENT boundary.
 */
MagicState magic_apply_padding(MagicFile* magic_file) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) {
        return MAGIC_ALIGNMENT_ERROR;
    }

    long current_offset = ftell(magic_file->model);
    if (current_offset < 0) {
        return MAGIC_ALIGNMENT_ERROR;
    }

    long padding_needed = (MAGIC_ALIGNMENT - (current_offset % MAGIC_ALIGNMENT)) % MAGIC_ALIGNMENT;

    if (magic_file->mode[0] == 'w') {
        // Writing mode: Pad with `0x00` bytes
        if (padding_needed > 0) {
            char padding[MAGIC_ALIGNMENT] = {0};
            if (fwrite(padding, 1, padding_needed, magic_file->model) != padding_needed) {
                return MAGIC_ALIGNMENT_ERROR;
            }
        }
    } else if (magic_file->mode[0] == 'r') {
        // Reading mode: Skip padding bytes
        if (fseek(magic_file->model, padding_needed, SEEK_CUR) != 0) {
            return MAGIC_ALIGNMENT_ERROR;
        }
    }

    return MAGIC_SUCCESS;
}

/**
 * @brief Writes a section marker and its size to the model file.
 */
MagicState magic_write_section_marker(MagicFile* magic_file, int64_t marker, int64_t section_size) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) {
        return MAGIC_ERROR;
    }

    if (magic_apply_padding(magic_file) != MAGIC_SUCCESS) {
        return MAGIC_ALIGNMENT_ERROR;
    }

    if (fwrite(&marker, sizeof(int64_t), 1, magic_file->model) != 1 ||
        fwrite(&section_size, sizeof(int64_t), 1, magic_file->model) != 1) {
        return MAGIC_ERROR;
    }

    return MAGIC_SUCCESS;
}

/**
 * @brief Reads a section marker and its size from the model file.
 */
MagicState magic_read_section_marker(MagicFile* magic_file, int64_t* marker, int64_t* section_size) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) {
        return MAGIC_ERROR;
    }

    if (magic_apply_padding(magic_file) != MAGIC_SUCCESS) {
        return MAGIC_ALIGNMENT_ERROR;
    }

    if (fread(marker, sizeof(int64_t), 1, magic_file->model) != 1 ||
        fread(section_size, sizeof(int64_t), 1, magic_file->model) != 1) {
        return MAGIC_ERROR;
    }

    return MAGIC_SUCCESS;
}

/**
 * @brief Writes the end marker (MAGIC_END) to the model file.
 */
MagicState magic_write_end_marker(MagicFile* magic_file) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) {
        return MAGIC_ERROR;
    }

    if (magic_apply_padding(magic_file) != MAGIC_SUCCESS) {
        return MAGIC_ALIGNMENT_ERROR;
    }

    int32_t magic = MAGIC_END;
    if (fwrite(&magic, sizeof(int32_t), 1, magic_file->model) != 1) {
        return MAGIC_ERROR;
    }

    return MAGIC_SUCCESS;
}

/**
 * @brief Reads and validates the end marker (MAGIC_END) from the model file.
 */
MagicState magic_read_end_marker(MagicFile* magic_file) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) {
        return MAGIC_ERROR;
    }

    if (magic_apply_padding(magic_file) != MAGIC_SUCCESS) {
        return MAGIC_ALIGNMENT_ERROR;
    }

    int32_t magic;
    if (fread(&magic, sizeof(int32_t), 1, magic_file->model) != 1 || magic != MAGIC_END) {
        LOG_ERROR("%s: Invalid end marker.\n", __func__);
        return MAGIC_ERROR;
    }

    return MAGIC_SUCCESS;
}
