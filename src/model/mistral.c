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

#define MISTRAL_FOREACH_GENERAL_FIELD \
    FIELD(model_type) \
    FIELD(model_base) \
    FIELD(author) \
    FIELD(created_at) \
    FIELD(last_modified) \
    FIELD(license) \
    FIELD(uuid)

MistralGeneral* mistral_read_general_section(MagicFile* magic_file) {
    const char* label = "general"; // Section label for logging

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

    #define READ_STRING(field) \
        MAGIC_READ_STRING(magic_file, general, field, label, mistral_free_general_section)

    // Read the general section fields
    #define FIELD(field) READ_STRING(field)
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

#define MISTRAL_FOREACH_PARAM_INT32_FIELD \
    FIELD(hidden_size) \
    FIELD(intermediate_size) \
    FIELD(max_position_embeddings) \
    FIELD(num_attention_heads) \
    FIELD(num_hidden_layers) \
    FIELD(num_key_value_heads) \
    FIELD(sliding_window) \
    FIELD(head_size)

#define MISTRAL_FOREACH_PARAM_FLOAT_FIELD \
    FIELD(rms_norm_eps) \
    FIELD(rope_theta) \
    FIELD(initializer_range)

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

    #define READ_INT32(field) \
        MAGIC_READ_INT32(magic_file, parameters, field, label, mistral_free_parameters_section)

    #define READ_FLOAT(field) \
        MAGIC_READ_FLOAT(magic_file, parameters, field, label, mistral_free_parameters_section);

    // Read fields using generalized macros
    MAGIC_READ_STRING(magic_file, parameters, hidden_act, label, mistral_free_parameters_section);
    MAGIC_READ_BOOL(magic_file, parameters, tie_word_embeddings, label, mistral_free_parameters_section);

    #define FIELD(field) READ_INT32(field)
    MISTRAL_FOREACH_PARAM_INT32_FIELD
    #undef FIELD
    #undef READ_INT32

    #define FIELD(field) READ_FLOAT(field)
    MISTRAL_FOREACH_PARAM_FLOAT_FIELD
    #undef FIELD

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

void mistral_log_parameters_section(MistralParameters* parameters) {
    #define LOG_INT32(field) \
        LOG_INFO("%s: Section: Parameters, Field: " #field "=%d\n", __func__, parameters->field);

    #define LOG_FLOAT(field) \
        LOG_INFO("%s: Section: Parameters, Field: " #field "=%.6f\n", __func__, (double) parameters->field);

    LOG_INFO("%s: Section: Parameters, Field: hidden_act=%s\n", __func__, parameters->hidden_act);
    LOG_INFO("%s: Section: Parameters, Field: tie_word_embeddings=%d\n", __func__, parameters->tie_word_embeddings);

    #define FIELD(field) LOG_INT32(field)
    MISTRAL_FOREACH_PARAM_INT32_FIELD
    #undef FIELD
    #undef LOG_INT32

    #define FIELD(field) LOG_FLOAT(field)
    MISTRAL_FOREACH_PARAM_FLOAT_FIELD
    #undef FIELD
}

Token* mistral_read_token(MagicFile* magic_file) {
    const char* label = "token"; // Section label for logging

    Token* token = (Token*) malloc(sizeof(Token));
    if (!token) {
        LOG_ERROR("%s: Failed to allocate memory for Token.\n", __func__);
        return NULL;
    }

    // Read string data
    MAGIC_READ_STRING(magic_file, token, data, label, mistral_free_token);
    token->length = (int32_t) strlen(token->data); // this is blocked if read string fails

    // Read token score
    MAGIC_READ_FLOAT(magic_file, token, score, label, mistral_free_token);

    // Read token type
    MAGIC_READ_INT32(magic_file, token, type, label, mistral_free_token);

    return token;
}

void mistral_free_token(Token* token) {
    if (token) {
        if (token->data) {
            free(token->data);
        }
        free(token);
    }
}

#define MISTRAL_FOREACH_TOKEN_INT32_FIELD \
    FIELD(vocab_size) \
    FIELD(bos_id) \
    FIELD(eos_id) \
    FIELD(pad_id) \
    FIELD(unk_id)

TokenizerModel* mistral_read_tokenizer_section(MagicFile* magic_file) {
    const char* label = "tokenizer"; // Section label for logging

    TokenizerModel* tokenizer = (TokenizerModel*) malloc(sizeof(TokenizerModel));
    if (!tokenizer) {
        LOG_ERROR("%s: Failed to allocate memory for TokenizerModel.\n", __func__);
        return NULL;
    }

    // Read the tokenizer section header
    int64_t marker = 0;
    int64_t size = 0;
    magic_file_read_section_marker(magic_file, &marker, &size);

    #define READ_INT32(field) \
        MAGIC_READ_INT32(magic_file, tokenizer, field, label, mistral_free_tokenizer_section)

    #define FIELD(field) READ_INT32(field)
    MISTRAL_FOREACH_TOKEN_INT32_FIELD
    #undef FIELD
    #undef READ_INT32

    // Mistrals vocab size (32000) is already predetermined
    if (tokenizer->vocab_size <= 0 || tokenizer->vocab_size > 32000) {
        LOG_ERROR("%s: Invalid vocab_size %d.\n", __func__, tokenizer->vocab_size);
        mistral_free_tokenizer_section(tokenizer);
        return NULL;
    }

    // Allocate memory for tokens array
    tokenizer->tokens = (Token**) malloc(tokenizer->vocab_size * sizeof(Token*));
    if (!tokenizer->tokens) {
        LOG_ERROR("%s: Failed to allocate memory for tokens array.\n", __func__);
        free(tokenizer);
        return NULL;
    }

    for (int32_t i = 0; i < tokenizer->vocab_size; i++) {
        LOG_DEBUG("%s: Reading token %d/%d.\n", __func__, i + 1, tokenizer->vocab_size);
        Token* token = mistral_read_token(magic_file);
        if (!token) {
            LOG_ERROR("%s: Failed to read token %d.\n", __func__, i + 1);
            mistral_free_tokenizer_section(tokenizer);
            return NULL;
        }
        tokenizer->tokens[i] = token;
    }

    // We must align the padding for the next section
    if (MAGIC_SUCCESS != magic_file_pad(magic_file)) {
        LOG_ERROR("%s: Failed to read alignment padding.\n", __func__);
        mistral_free_tokenizer_section(tokenizer);
        return NULL;
    }

    return tokenizer;
}

void mistral_free_tokenizer_section(TokenizerModel* tokenizer) {
    if (tokenizer) {
        if (tokenizer->tokens) {
            for (int32_t i = 0; i < tokenizer->vocab_size; i++) {
                mistral_free_token(tokenizer->tokens[i]);
            }
            free(tokenizer->tokens);
        }
        free(tokenizer);
    }
}

void mistral_log_tokenizer_section(TokenizerModel* tokenizer) {
    #define LOG_INT32(field) \
        LOG_INFO("%s: Section: Tokenizer, Field: " #field "=%d\n", __func__, tokenizer->field);

    #define FIELD(field) LOG_INT32(field)
    MISTRAL_FOREACH_TOKEN_INT32_FIELD
    #undef FIELD
    #undef LOG_INT32

    LOG_INFO("%s: Tokenizer: %p", __func__, tokenizer->tokens);
    for (int32_t i = 0; i < tokenizer->vocab_size; i++) {
        Token* token = tokenizer->tokens[i];
        LOG_INFO(
            "%s: Token: token=%p, length=%d, data=%s, score=%.6f, type=%d\n",
            __func__, token, token->length, token->data, (double) token->score, token->type
        );
    }
}
