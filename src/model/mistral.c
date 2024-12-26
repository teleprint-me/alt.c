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

    // Read the general section fields
    if (MAGIC_SUCCESS != magic_file_read_string_field(magic_file, &mistral_general->model_type)) {
        LOG_ERROR("%s: Failed to read model_type from general section.\n", __func__);
        mistral_free_general_section(mistral_general);
        return NULL;
    }
    if (MAGIC_SUCCESS != magic_file_read_string_field(magic_file, &mistral_general->model_base)) {
        LOG_ERROR("%s: Failed to read model_base from general section.\n", __func__);
        mistral_free_general_section(mistral_general);
        return NULL;
    }
    if (MAGIC_SUCCESS != magic_file_read_string_field(magic_file, &mistral_general->author)) {
        LOG_ERROR("%s: Failed to read author from general section.", __func__);
        mistral_free_general_section(mistral_general);
        return NULL;
    }
    if (MAGIC_SUCCESS != magic_file_read_string_field(magic_file, &mistral_general->created_at)) {
        LOG_ERROR("%s: Failed to read created_at from general section.", __func__);
        mistral_free_general_section(mistral_general);
        return NULL;
    }
    if (MAGIC_SUCCESS
        != magic_file_read_string_field(magic_file, &mistral_general->last_modified)) {
        LOG_ERROR("%s: Failed to read last_modified from general section.", __func__);
        mistral_free_general_section(mistral_general);
        return NULL;
    }
    if (MAGIC_SUCCESS != magic_file_read_string_field(magic_file, &mistral_general->license)) {
        LOG_ERROR("%s: Failed to read license from general section.", __func__);
        mistral_free_general_section(mistral_general);
        return NULL;
    }
    if (MAGIC_SUCCESS != magic_file_read_string_field(magic_file, &mistral_general->uuid)) {
        LOG_ERROR("%s: Failed to read uuid from general section.", __func__);
        mistral_free_general_section(mistral_general);
        return NULL;
    }

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
        if (mistral_general->uuid) {
            free(mistral_general->uuid);
        }
        if (mistral_general->license) {
            free(mistral_general->license);
        }
        if (mistral_general->last_modified) {
            free(mistral_general->last_modified);
        }
        if (mistral_general->created_at) {
            free(mistral_general->created_at);
        }
        if (mistral_general->author) {
            free(mistral_general->author);
        }
        if (mistral_general->model_base) {
            free(mistral_general->model_base);
        }
        if (mistral_general->model_type) {
            free(mistral_general->model_type);
        }
        // free the section structure
        free(mistral_general);
    }
}
