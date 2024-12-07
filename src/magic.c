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

// Function definitions
FILE* magic_file_open(MagicFile* magic_file) {
    if (magic_file == NULL) { return NULL; }
    magic_file->model = fopen(magic_file->filepath, magic_file->mode);
    if (magic_file->model == NULL) {
        LOG_ERROR("%s: Unable to open file %s\n", __func__, magic_file->filepath);
    }
    return magic_file->model;
}

MagicState magic_file_validate(MagicFile* magic_file) {
    if (magic_file == NULL || magic_file->model == NULL) {
        LOG_ERROR("%s: Invalid file pointer.\n", __func__);
        return MAGIC_ERROR;
    }
    if (access(magic_file->filepath, F_OK | (magic_file->mode[0] == 'r' ? R_OK : W_OK)) != 0) {
        LOG_ERROR("%s: File %s is inaccessible or does not meet the required permissions.\n", __func__, magic_file->filepath);
        return MAGIC_ERROR;
    }

    // For read access, validate ALT magic number
    if (magic_file->mode[0] == 'r') {
        int32_t magic;
        if (fread(&magic, sizeof(int32_t), 1, magic_file->model) != 1) {
            LOG_ERROR("%s: Failed to read magic number from file %s\n", __func__, magic_file->filepath);
            fclose(magic_file->model);
            return MAGIC_ERROR;
        }

        // Validate the magic number
        if (magic != MAGIC_ALT) {
            LOG_ERROR("%s: Invalid ALT file (magic number mismatch) for file %s\n", __func__, magic_file->filepath);
            fclose(magic_file->model);
            return MAGIC_ERROR;
        }
        // Reset file pointer to the beginning for further operations
        fseek(magic_file->model, 0, SEEK_SET);
    }
    return MAGIC_SUCCESS;
}

void magic_file_close(MagicFile* magic_file) {
    if (magic_file && magic_file->model) {
        fclose(magic_file->model);
        magic_file->model = NULL;
    }
}

// Constructor function for MagicFile
MagicFile magic_file_create(const char* filepath, const char* mode) {
    return (MagicFile) {
        .filepath = filepath,
        .mode = mode,
        .model = NULL, // pointer to the model file
        .open = magic_file_open,
        .validate = magic_file_validate,
        .close = magic_file_close
    };
}

// Helper function for guarding write and read functions
MagicState magic_file_guard(MagicFile* magic_file) {
    if (magic_file == NULL || magic_file->model == NULL) {
        LOG_ERROR("%s: Invalid file pointer.\n", __func__);
        return MAGIC_ERROR;
    }
    return MAGIC_SUCCESS;
}

// Helper function for calculating the section offset
long magic_calculate_align_offset(MagicFile* magic_file) {
    return (MAGIC_ALIGNMENT - (ftell(magic_file->model) % MAGIC_ALIGNMENT)) % MAGIC_ALIGNMENT;
}

// Write functions

// Function to align the file pointer by writing padding up to the MAGIC_ALIGNMENT boundary
void magic_write_align_offset(MagicFile* magic_file) {
    // Calculate the padding needed based on the current file offset
    long padding_needed = magic_calculate_align_offset(magic_file);
    
    // Write zero bytes to align if padding is required
    if (padding_needed > 0) {
        char padding[MAGIC_ALIGNMENT] = {0};  // Allocate a buffer with zeros
        fwrite(padding, 1, padding_needed, magic_file->model);
        printf("Aligned offset with %ld bytes of padding.\n", padding_needed);
    }
}

MagicState magic_write_start_marker(MagicFile* magic_file) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) { return MAGIC_ERROR; }
    int32_t magic = MAGIC_ALT;
    fwrite(&magic, sizeof(int32_t), 1, magic_file->model);
    return MAGIC_SUCCESS;
}

MagicState magic_write_end_marker(MagicFile* magic_file) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) { return MAGIC_ERROR; }
    magic_write_align_offset(magic_file);  // Align before writing the end marker
    int32_t magic = MAGIC_END;
    fwrite(&magic, sizeof(int32_t), 1, magic_file->model);
    return MAGIC_SUCCESS;
}

MagicState magic_write_section_marker(MagicFile* magic_file, int64_t marker, int64_t section_size) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) { return MAGIC_ERROR; }
    magic_write_align_offset(magic_file);  // Align before writing section marker
    fwrite(&marker, sizeof(int64_t), 1, magic_file->model);
    fwrite(&section_size, sizeof(int64_t), 1, magic_file->model);
    return MAGIC_SUCCESS;
}

// Read functions

// Function to align the file pointer to the nearest ALIGNMENT boundary
void magic_read_align_offset(MagicFile* magic_file) {
    // Calculate the padding needed based on the current file offset
    long padding_needed = magic_calculate_align_offset(magic_file);

    // Read zero bytes to align if padding is required
    if (padding_needed > 0) {
        fseek(magic_file->model, padding_needed, SEEK_CUR);
        LOG_DEBUG("%s: Aligned offset with %ld bytes of padding.\n", __func__, padding_needed);
    }
}

MagicState magic_read_start_marker(MagicFile* magic_file) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) { return MAGIC_ERROR; }

    long offset = ftell(magic_file->model);
    LOG_DEBUG("%s: Reading start marker: %ld\n", __func__, offset);

    int32_t magic;
    fread(&magic, sizeof(int32_t), 1, magic_file->model);
    if (magic != MAGIC_ALT) {
        LOG_ERROR("%s: Invalid start marker (expected 0x%x, got 0x%x)\n", __func__, MAGIC_ALT, magic);
        return MAGIC_ERROR;
    }

    offset = ftell(magic_file->model);
    LOG_DEBUG("%s: Start marker offset: %ld,  Magic: 0x%x\n\n", __func__, offset, magic);

    return MAGIC_SUCCESS;
}

MagicState magic_read_end_marker(MagicFile* magic_file) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) { return MAGIC_ERROR; }

    long offset = ftell(magic_file->model);
    LOG_DEBUG("%s: Reading end marker: %ld\n", __func__, offset);

    magic_read_align_offset(magic_file); // Align the marker
    long aligned_offset = ftell(magic_file->model);
    LOG_DEBUG("%s: Aligned Offset: %ld\n", __func__, aligned_offset);

    int32_t magic;
    fread(&magic, sizeof(int32_t), 1, magic_file->model);
    if (magic != MAGIC_END) {
        LOG_ERROR("%s: Invalid end marker (expected 0x%x, got 0x%x)\n", __func__, MAGIC_END, magic);
        return MAGIC_ERROR;
    }

    offset = ftell(magic_file->model);  // Update offset after alignment
    LOG_DEBUG("%s: End marker offset: %ld, Magic: 0x%x\n", __func__, offset, magic);

    return MAGIC_SUCCESS;
}

MagicState magic_read_section_marker(MagicFile* magic_file, int64_t* marker, int64_t* section_size) {
    if (magic_file_guard(magic_file) == MAGIC_ERROR) {
        return MAGIC_ERROR;
    }

    // Initial offset for debugging
    long initial_offset = ftell(magic_file->model);
    LOG_DEBUG("%s: Reading section marker: %ld\n", __func__, initial_offset);

    // Align the file position and update the offset after alignment
    magic_read_align_offset(magic_file);
    long aligned_offset = ftell(magic_file->model);
    LOG_DEBUG("%s: Aligned Offset: %ld\n", __func__, aligned_offset);

    // Read the section marker and size, checking for read success
    if (fread(marker, sizeof(int64_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read section marker at offset %ld.\n", __func__, aligned_offset);
        return MAGIC_ERROR;
    }

    if (fread(section_size, sizeof(int64_t), 1, magic_file->model) != 1) {
        LOG_ERROR("%s: Failed to read section size at offset %ld.\n", __func__, ftell(magic_file->model));
        return MAGIC_ERROR;
    }

    // Current offset after reading marker and section size
    long end_offset = ftell(magic_file->model);
    LOG_DEBUG("%s: Section Marker: 0x%lx, Section Size: %ld, End Offset: %ld\n", __func__, *marker, *section_size, end_offset);

    // Validate the section marker against known values
    switch (*marker) {
        case MAGIC_GENERAL:
        case MAGIC_PARAMETERS:
        case MAGIC_TOKENIZER:
        case MAGIC_TENSORS:
            LOG_DEBUG("%s: Valid Section Marker found: 0x%lx\n", __func__, *marker);
            break;
        default:
            LOG_ERROR("%s: Invalid section marker 0x%lx at offset %ld.\n", __func__, *marker, aligned_offset);
            return MAGIC_ERROR;
    }

    return MAGIC_SUCCESS;
}
