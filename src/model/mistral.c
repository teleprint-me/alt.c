/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/model/mistral.c
 *
 * See specification for more details about the Altiera model file format.
 * @ref docs/model/specification.md
 *
 * @note Mistrals tokenizer is a sentencepiece byte-pair encoding model. Unsure of how I'd like to
 * handle this at the moment. Will figure it out later.
 */

#include <stdbool.h>

#include "logger.h"
#include "model/magic.h"
#include "model/mistral.h"

MistralMagic* mistral_read_start_section(MagicFile* magic_file) {
    // Allocate memory for start section
    MistralMagic* mistral_magic = (MistralMagic*) malloc(sizeof(MistralMagic));
    if (!mistral_magic) {
        LOG_ERROR("%s: Failed to allocate memory to MistralMagic.\n", __func__);
        return NULL;
    }

    // Set default values
    mistral_magic->version = MAGIC_VERSION;
    mistral_magic->alignment = MAGIC_ALIGNMENT;

    // Read the start section (note that this aligns padding for us)
    magic_file_read_start_marker(magic_file, &mistral_magic->version, &mistral_magic->alignment);
    return mistral_magic;
}

void mistral_free_start_section(MistralMagic* mistral_magic) {
    if (mistral_magic) {
        free(mistral_magic);
    }
}

MistralGeneral* mistral_read_general_section(MagicFile* magic_file) {
    // Allocate memory for general section
    MistralGeneral* mistral_general = (MistralGeneral*) malloc(sizeof(MistralGeneral));
    if (!mistral_general) {
        LOG_ERROR("%s: Failed to allocate memory to MistralGeneral.\n", __func__);
        return NULL;
    }

    // Read the general section header
    int64_t general_marker = 0;
    int64_t general_size = 0;
    magic_file_read_section_marker(magic_file, &general_marker, &general_size);

    #define READ_FIELD(field) \
        if (MAGIC_SUCCESS != magic_file_read_string_field(magic_file, &mistral_general->field)) { \
            LOG_ERROR("Failed to read " #field " from general section."); \
            mistral_free_general_section(mistral_general); \
            return NULL; \
        }

    // Read the general section fields
    READ_FIELD(model_type);
    READ_FIELD(model_base);
    READ_FIELD(author);
    READ_FIELD(created_at);
    READ_FIELD(last_modified);
    READ_FIELD(license);
    READ_FIELD(uuid);

    // We must align the padding for the next section
    if (MAGIC_SUCCESS != magic_file_pad(magic_file)) {
        LOG_ERROR("%s: Failed to read alignment padding.\n", __func__);
        mistral_free_general_section(mistral_general);
        return NULL;
    }

    // Return the models section
    return mistral_general;
}

void mistral_free_general_section(MistralGeneral* mistral_general) {
    if (mistral_general) {
        // free strings first
        #define FREE_FIELD(field) \
            if (mistral_general->field) { \
                free(mistral_general->field); \
            }

        FREE_FIELD(model_type);
        FREE_FIELD(model_base);
        FREE_FIELD(author);
        FREE_FIELD(created_at);
        FREE_FIELD(last_modified);
        FREE_FIELD(license);
        FREE_FIELD(uuid);

        // free the section structure
        free(mistral_general);
    }
}

void mistral_log_general_section(MistralGeneral* mistral_general) {
    #define LOG_FIELD(field) \
        LOG_INFO("%s: Section: General, Field: " #field "=%s\n", __func__, mistral_general->field);

    LOG_FIELD(model_type);
    LOG_FIELD(model_base);
    LOG_FIELD(author);
    LOG_FIELD(created_at);
    LOG_FIELD(last_modified);
    LOG_FIELD(license);
    LOG_FIELD(uuid);
}
