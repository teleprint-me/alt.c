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
    MistralMagic* header = (MistralMagic*) malloc(sizeof(MistralMagic));
    if (!header) {
        LOG_ERROR("%s: Failed to allocate memory to MistralMagic.\n", __func__);
        return NULL;
    }

    // Set default values
    header->version = MAGIC_VERSION;
    header->alignment = MAGIC_ALIGNMENT;

    // Read the start section (note that this aligns padding for us)
    magic_file_read_start_marker(magic_file, &header->version, &header->alignment);
    return header;
}

void mistral_free_start_section(MistralMagic* header) {
    if (header) {
        free(header);
    }
}

/**
 * This macro-driven approach is specific to the general section,
 * which is uniform (all fields are char*). It simplifies the logic
 * for reading, logging, and freeing fields. While it’s currently
 * unique to this section, it may serve as a reference or template
 * for similar patterns in future code.
 */
#define MISTRAL_FOREACH_GENERAL_FIELD \
    FIELD(model_type) \
    FIELD(model_base) \
    FIELD(author) \
    FIELD(created_at) \
    FIELD(last_modified) \
    FIELD(license) \
    FIELD(uuid)

MistralGeneral* mistral_read_general_section(MagicFile* magic_file) {
    // Allocate memory for general section
    MistralGeneral* general = (MistralGeneral*) malloc(sizeof(MistralGeneral));
    if (!general) {
        LOG_ERROR("%s: Failed to allocate memory to MistralGeneral.\n", __func__);
        return NULL;
    }

    // Read the general section header
    int64_t marker = 0;
    int64_t size = 0;
    magic_file_read_section_marker(magic_file, &marker, &size);

    #define READ_FIELD(field) \
        if (MAGIC_SUCCESS != magic_file_read_string_field(magic_file, &general->field)) { \
            LOG_ERROR("Failed to read " #field " from general section."); \
            mistral_free_general_section(general); \
            return NULL; \
        }

    // Read the general section fields
    #define FIELD(field) READ_FIELD(field)
    MISTRAL_FOREACH_GENERAL_FIELD
    #undef FIELD

    // We must align the padding for the next section
    if (MAGIC_SUCCESS != magic_file_pad(magic_file)) {
        LOG_ERROR("%s: Failed to read alignment padding.\n", __func__);
        mistral_free_general_section(general);
        return NULL;
    }

    // Return the models section
    return general;
}

void mistral_free_general_section(MistralGeneral* general) {
    if (general) {
        // free strings first
        #define FREE_FIELD(field) \
            if (general->field) { \
                free(general->field); \
            }

        #define FIELD(field) FREE_FIELD(field)
        MISTRAL_FOREACH_GENERAL_FIELD
        #undef FIELD

        // free the section structure
        free(general);
    }
}

void mistral_log_general_section(MistralGeneral* general) {
    #define LOG_FIELD(field) \
        LOG_INFO("%s: Section: General, Field: " #field "=%s\n", __func__, general->field);

    #define FIELD(field) LOG_FIELD(field)
    MISTRAL_FOREACH_GENERAL_FIELD
    #undef FIELD
}

MistralParameters* mistral_read_parameters_section(MagicFile* magic_file) {
    const char* label = "parameters"; // Section label for logging

    MistralParameters* parameters = (MistralParameters*) malloc(sizeof(MistralParameters));
    if (!parameters) {
        LOG_ERROR("%s: Failed to allocate memory for MistralParameters.\n", __func__);
        return NULL;
    }

    // Read the parameters section header
    int64_t marker = 0;
    int64_t size = 0;
    magic_file_read_section_marker(magic_file, &marker, &size);

    // Read fields using generalized macros
    MAGIC_READ_STRING(magic_file, parameters, hidden_act, label, mistral_free_parameters_section);
    MAGIC_READ_BOOL(magic_file, parameters, tie_word_embeddings, label, mistral_free_parameters_section);
    MAGIC_READ_INT32(magic_file, parameters, hidden_size, label, mistral_free_parameters_section);
    MAGIC_READ_INT32(magic_file, parameters, intermediate_size, label, mistral_free_parameters_section);
    MAGIC_READ_INT32(magic_file, parameters, max_position_embeddings, label, mistral_free_parameters_section);
    MAGIC_READ_INT32(magic_file, parameters, num_attention_heads, label, mistral_free_parameters_section);
    MAGIC_READ_INT32(magic_file, parameters, num_hidden_layers, label, mistral_free_parameters_section);
    MAGIC_READ_INT32(magic_file, parameters, num_key_value_heads, label, mistral_free_parameters_section);
    MAGIC_READ_INT32(magic_file, parameters, sliding_window, label, mistral_free_parameters_section);
    MAGIC_READ_FLOAT(magic_file, parameters, rope_theta, label, mistral_free_parameters_section);
    MAGIC_READ_FLOAT(magic_file, parameters, rms_norm_eps, label, mistral_free_parameters_section);
    MAGIC_READ_FLOAT(magic_file, parameters, initializer_range, label, mistral_free_parameters_section);

    // We must align the padding for the next section
    if (MAGIC_SUCCESS != magic_file_pad(magic_file)) {
        LOG_ERROR("%s: Failed to read alignment padding.\n", __func__);
        mistral_free_parameters_section(parameters);
        return NULL;
    }

    return parameters;
}

void mistral_free_parameters_section(MistralParameters* parameters) {
    if (parameters) {
        if (parameters->hidden_act) {
            free(parameters->hidden_act); // I think this would be the only allocated field here
        }
        free(parameters); // just free the struct since we only allocate mem to 1 field
    }
}
