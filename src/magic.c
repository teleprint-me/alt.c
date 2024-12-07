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
    LOG_DEBUG("%s: MagicFile created for %s in mode %s.\n", __func__, filepath, mode);
    return magic_file;
}

/**
 * @brief Opens the model file based on the MagicFile structure.
 */
MagicState magic_file_open(MagicFile* magic_file) {
    if (magic_file == NULL) {
        LOG_ERROR("%s: MagicFile is NULL.\n", __func__);
        return MAGIC_ERROR;
    }
    magic_file->model = fopen(magic_file->filepath, magic_file->mode);
    if (magic_file->model == NULL) {
        LOG_ERROR("%s: Unable to open file %s\n", __func__, magic_file->filepath);
        return MAGIC_FILE_ERROR;
    }
    LOG_DEBUG("%s: File %s opened successfully.\n", __func__, magic_file->filepath);
    return MAGIC_SUCCESS;
}

/**
 * @brief Closes the model file.
 */
void magic_file_close(MagicFile* magic_file) {
    if (magic_file && magic_file->model) {
        fclose(magic_file->model);
        magic_file->model = NULL;
        LOG_DEBUG("%s: File closed successfully.\n", __func__);
    }
}

/**
 * @brief Validates the model file, checking existence, permissions, and the header.
 */
MagicState magic_file_validate(MagicFile* magic_file) {
    if (magic_file == NULL || magic_file->model == NULL) {
        LOG_ERROR("%s: Invalid MagicFile structure or file pointer is NULL.\n", __func__);
        return MAGIC_ERROR;
    }

    // Read and validate the Start Marker
    int32_t magic_version = 0;
    int32_t magic_alignment = 0;
    MagicState state = magic_read_start_marker(magic_file, &magic_version, &magic_alignment);
    if (state != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Start Marker validation failed.\n", __func__);
        return state;
    }

    // Reset the file pointer to the start
    if (fseek(magic_file->model, 0, SEEK_SET) != 0) {
        LOG_ERROR("%s: Failed to reset the file pointer after validation.\n", __func__);
        return MAGIC_ERROR;
    }

    LOG_DEBUG("%s: Model file validated successfully. Version: %d, Alignment: %d.\n",
              __func__, magic_version, magic_alignment);
    return MAGIC_SUCCESS;
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
        LOG_ERROR("%s: Failed to get file offset.\n", __func__);
        return MAGIC_ALIGNMENT_ERROR;
    }

    size_t padding_needed = (MAGIC_ALIGNMENT - (current_offset % MAGIC_ALIGNMENT)) % MAGIC_ALIGNMENT;

    if (magic_file->mode[0] == 'w') {
        // Writing mode: Pad with `0x00` bytes
        if (padding_needed > 0) {
            char padding[MAGIC_ALIGNMENT] = {0};
            if (fwrite(padding, 1, padding_needed, magic_file->model) != padding_needed) {
                LOG_ERROR("%s: Failed to write padding bytes.\n", __func__);
                return MAGIC_ALIGNMENT_ERROR;
            }
            LOG_DEBUG("%s: Wrote %ld padding bytes.\n", __func__, padding_needed);
        }
    } else if (magic_file->mode[0] == 'r') {
        // Reading mode: Skip padding bytes
        if (fseek(magic_file->model, padding_needed, SEEK_CUR) != 0) {
            LOG_ERROR("%s: Failed to skip padding bytes.\n", __func__);
            return MAGIC_ALIGNMENT_ERROR;
        }
        LOG_DEBUG("%s: Skipped %ld padding bytes.\n", __func__, padding_needed);
    }

    return MAGIC_SUCCESS;
}

MagicState magic_write_start_marker(MagicFile* magic_file, int32_t version, int32_t alignment) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) {
        return MAGIC_ERROR;
    }

    // Define the fields for the Start Marker section
    int64_t section_marker = MAGIC_ALT;
    int64_t section_size
        = sizeof(int64_t) + sizeof(int64_t) + sizeof(int32_t) + sizeof(int32_t); // 24 bytes

    // Write the Start Marker section fields
    if (fwrite(&section_marker, sizeof(int64_t), 1, magic_file->model) != 1
        || fwrite(&section_size, sizeof(int64_t), 1, magic_file->model) != 1
        || fwrite(&version, sizeof(int32_t), 1, magic_file->model) != 1
        || fwrite(&alignment, sizeof(int32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write Start Marker section.\n", __func__);
        return MAGIC_ERROR;
    }

    LOG_DEBUG(
        "%s: Start Marker written successfully. Marker: 0x%lx, Size: %ld, Version: %d, Alignment: "
        "%d.\n",
        __func__,
        section_marker,
        section_size,
        version,
        alignment
    );
    return MAGIC_SUCCESS;
}

MagicState magic_read_start_marker(MagicFile* magic_file, int32_t* version, int32_t* alignment) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) {
        return MAGIC_ERROR;
    }

    // Define the fields for the Start Marker section
    int64_t section_marker = 0;
    int64_t section_size = 0;
    int32_t magic_version = 0;
    int32_t magic_alignment = 0;

    // Read the Start Marker section fields
    if (fread(&section_marker, sizeof(int64_t), 1, magic_file->model) != 1
        || fread(&section_size, sizeof(int64_t), 1, magic_file->model) != 1
        || fread(&magic_version, sizeof(int32_t), 1, magic_file->model) != 1
        || fread(&magic_alignment, sizeof(int32_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read Start Marker section.\n", __func__);
        return MAGIC_ERROR;
    }

    // Validate the section marker
    if (section_marker != MAGIC_ALT) {
        LOG_ERROR(
            "%s: Invalid Start Marker. Expected 0x%lx, got 0x%lx.\n",
            __func__,
            MAGIC_ALT,
            section_marker
        );
        return MAGIC_INVALID_MARKER;
    }

    // Validate the section size
    const int64_t expected_size
        = sizeof(int64_t) + sizeof(int64_t) + sizeof(int32_t) + sizeof(int32_t); // 24 bytes
    if (section_size != expected_size) {
        LOG_ERROR(
            "%s: Invalid Start Marker size. Expected %ld, got %ld.\n",
            __func__,
            expected_size,
            section_size
        );
        return MAGIC_ERROR;
    }

    // Return the version and alignment
    if (version != NULL) {
        *version = magic_version;
    }
    if (alignment != NULL) {
        *alignment = magic_alignment;
    }

    LOG_DEBUG(
        "%s: Start Marker read successfully. Marker: 0x%lx, Size: %ld, Version: %d, Alignment: "
        "%d.\n",
        __func__,
        section_marker,
        section_size,
        magic_version,
        magic_alignment
    );
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

    if (fwrite(&marker, sizeof(int64_t), 1, magic_file->model) != 1
        || fwrite(&section_size, sizeof(int64_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to write section marker or size.\n", __func__);
        return MAGIC_ERROR;
    }

    LOG_DEBUG("%s: Wrote section marker 0x%lx with size %ld.\n", __func__, marker, section_size);
    return MAGIC_SUCCESS;
}

/**
 * @brief Reads a section marker and its size from the model file.
 */
MagicState
magic_read_section_marker(MagicFile* magic_file, int64_t* marker, int64_t* section_size) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) {
        return MAGIC_ERROR;
    }

    if (magic_apply_padding(magic_file) != MAGIC_SUCCESS) {
        return MAGIC_ALIGNMENT_ERROR;
    }

    if (fread(marker, sizeof(int64_t), 1, magic_file->model) != 1
        || fread(section_size, sizeof(int64_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read section marker or size.\n", __func__);
        return MAGIC_ERROR;
    }

    LOG_DEBUG("%s: Read section marker 0x%lx with size %ld.\n", __func__, *marker, *section_size);
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
        LOG_ERROR("%s: Failed to write end marker.\n", __func__);
        return MAGIC_ERROR;
    }

    LOG_DEBUG("%s: Wrote end marker.\n", __func__);
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

    LOG_DEBUG("%s: Read and validated end marker.\n", __func__);
    return MAGIC_SUCCESS;
}
