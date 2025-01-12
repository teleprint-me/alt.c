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

#include "interface/logger.h"

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
#undef READ_STRING

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
#undef FREE_FIELD

        // free the section structure
        free(general);
    }
}

void mistral_log_general_section(MistralGeneral* general){
#define LOG_FIELD(field) \
    LOG_DEBUG("%s: Section: General, Field: " #field "=%s\n", __func__, general->field);

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
    MAGIC_READ_BOOL(
        magic_file, parameters, tie_word_embeddings, label, mistral_free_parameters_section
    );

#define FIELD(field) READ_INT32(field)
    MISTRAL_FOREACH_PARAM_INT32_FIELD
#undef FIELD
#undef READ_INT32

#define FIELD(field) READ_FLOAT(field)
    MISTRAL_FOREACH_PARAM_FLOAT_FIELD
#undef FIELD
#undef READ_FLOAT

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
    LOG_DEBUG("%s: Section: Parameters, Field: " #field "=%d\n", __func__, parameters->field);

#define LOG_FLOAT(field) \
    LOG_DEBUG( \
        "%s: Section: Parameters, Field: " #field "=%.6f\n", __func__, (double) parameters->field \
    );

    LOG_DEBUG("%s: Section: Parameters, Field: hidden_act=%s\n", __func__, parameters->hidden_act);
    LOG_DEBUG(
        "%s: Section: Parameters, Field: tie_word_embeddings=%d\n",
        __func__,
        parameters->tie_word_embeddings
    );

#define FIELD(field) LOG_INT32(field)
    MISTRAL_FOREACH_PARAM_INT32_FIELD
#undef FIELD
#undef LOG_INT32

#define FIELD(field) LOG_FLOAT(field)
    MISTRAL_FOREACH_PARAM_FLOAT_FIELD
#undef FIELD
#undef LOG_FLOAT
}

Token* mistral_read_token(MagicFile* magic_file) {
    const char* label = "token"; // Section label for logging

    Token* token = (Token*) malloc(sizeof(Token));
    if (!token) {
        LOG_ERROR("%s: Failed to allocate memory for Token.\n", __func__);
        return NULL;
    }

    // Read token fields
    MAGIC_READ_FLOAT(magic_file, token, score, label, mistral_free_token);
    MAGIC_READ_INT32(magic_file, token, type, label, mistral_free_token);
    MAGIC_READ_INT32(magic_file, token, id, label, mistral_free_token);
    MAGIC_READ_STRING(magic_file, token, data, label, mistral_free_token);

    token->length = (int32_t) strlen(token->data);

    LOG_DEBUG(
        "%s: Token: %p, score=%.6f, type=%d, id=%d, length=%d, data=%s\n",
        __func__,
        token,
        (double) token->score,
        token->type,
        token->id,
        token->length,
        token->data
    );

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

HashTableState mistral_add_token_to_table(TokenizerModel* model, Token* token) {
    if (!model || !token || !token->data || token->length <= 0) {
        LOG_ERROR("%s: Invalid arguments.\n", __func__);
        return HASH_ERROR;
    }

    // Check for duplicate token string
    int32_t* existing_id = (int32_t*) hash_search(model->table, token->data);
    if (existing_id) {
        LOG_ERROR(
            "%s: Duplicate token detected in HashTable. Existing data: '%s', New data: '%s'\n",
            __func__,
            model->tokens[*existing_id]->data,
            token->data
        );
        return HASH_KEY_EXISTS;
    }

    // Insert string -> Add token as key and id as value to enable reverse lookups
    if (hash_insert(model->table, token->data, &token->id) != HASH_SUCCESS) {
        LOG_ERROR("%s: Failed to insert token string -> ID into HashTable.\n", __func__);
        return HASH_ERROR;
    }

    return HASH_SUCCESS;
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
    tokenizer->tokens = NULL;
    tokenizer->table = NULL;

    // Read the tokenizer section header
    int64_t marker = 0, size = 0;
    if (magic_file_read_section_marker(magic_file, &marker, &size) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to read tokenizer section marker.\n", __func__);
        mistral_free_tokenizer_section(tokenizer);
        return NULL;
    }

#define READ_INT32(field) \
    MAGIC_READ_INT32(magic_file, tokenizer, field, label, mistral_free_tokenizer_section)

#define FIELD(field) READ_INT32(field)
    MISTRAL_FOREACH_TOKEN_INT32_FIELD
#undef FIELD
#undef READ_INT32

    // Mistrals vocab size is 32000
    if (tokenizer->vocab_size <= 0 || tokenizer->vocab_size > 32000) {
        LOG_ERROR("%s: Invalid vocab_size: %d.\n", __func__, tokenizer->vocab_size);
        mistral_free_tokenizer_section(tokenizer);
        return NULL;
    }

    // Allocate tokens array
    tokenizer->tokens = (Token**) calloc(tokenizer->vocab_size, sizeof(Token*));
    if (!tokenizer->tokens) {
        LOG_ERROR("%s: Failed to allocate tokens array.\n", __func__);
        mistral_free_tokenizer_section(tokenizer);
        return NULL;
    }

    // Create the hash table
    tokenizer->table = hash_create_table(tokenizer->vocab_size, HASH_TYPE_STRING);
    if (!tokenizer->table) {
        LOG_ERROR("%s: Failed to create hash table for tokenizer.\n", __func__);
        mistral_free_tokenizer_section(tokenizer);
        return NULL;
    }

    // Read tokens
    for (int32_t i = 0; i < tokenizer->vocab_size; i++) {
        Token* token = mistral_read_token(magic_file);
        if (!token) {
            LOG_ERROR("%s: Failed to read token at index %d.\n", __func__, i);
            mistral_free_tokenizer_section(tokenizer);
            return NULL;
        }

        // Add token to the table
        if (mistral_add_token_to_table(tokenizer, token) != HASH_SUCCESS) {
            LOG_ERROR(
                "%s: Failed to add token to table. Token: '%s', ID: %d\n",
                __func__,
                token->data,
                token->id
            );
            mistral_free_token(token);
            mistral_free_tokenizer_section(tokenizer);
            return NULL;
        }

        // Store token in the array
        tokenizer->tokens[token->id] = token;
    }

    // Align for next section
    if (magic_file_pad(magic_file) != MAGIC_SUCCESS) {
        LOG_ERROR("%s: Failed to align tokenizer section.\n", __func__);
        mistral_free_tokenizer_section(tokenizer);
        return NULL;
    }

    return tokenizer;
}

void mistral_free_tokenizer_section(TokenizerModel* tokenizer) {
    if (tokenizer) {
        if (tokenizer->table) {
            hash_clear(tokenizer->table);
            hash_free_table(tokenizer->table);
        }

        if (tokenizer->tokens) {
            for (int32_t i = 0; i < tokenizer->vocab_size; i++) {
                if (tokenizer->tokens[i]) {
                    mistral_free_token(tokenizer->tokens[i]);
                }
            }
            free(tokenizer->tokens);
        }

        free(tokenizer);
    }
}

void mistral_log_tokenizer_section(TokenizerModel* tokenizer) {
// Log integer fields of the tokenizer
#define LOG_INT32(field) \
    LOG_DEBUG("%s: Section: Tokenizer, Field: " #field "=%d\n", __func__, tokenizer->field);

#define FIELD(field) LOG_INT32(field)
    MISTRAL_FOREACH_TOKEN_INT32_FIELD
#undef FIELD
#undef LOG_INT32

    // Log tokens from the hash map
    LOG_DEBUG("%s: Tokenizer contains %d tokens.\n", __func__, tokenizer->vocab_size);

    for (int32_t i = 0; i < tokenizer->vocab_size; i++) {
        Token* token = tokenizer->tokens[i];
        LOG_DEBUG(
            "%s: Token: %p, score=%.6f, type=%d, id=%d, length=%d, data=%s\n",
            __func__,
            token,
            (double) token->score,
            token->type,
            token->id,
            token->length,
            token->data
        );
    }
}

// Reverse lookup (requires using the hash table)
int32_t mistral_get_id_by_token(TokenizerModel* tokenizer, const char* data) {
    if (!tokenizer || !data) {
        LOG_ERROR("%s: Invalid arguments.\n", __func__);
        return -1; // Use -1 to indicate failure
    }

    int32_t* id = (int32_t*) hash_search(tokenizer->table, data);
    if (!id) {
        LOG_WARN("%s: Token '%s' not found.\n", __func__, data);
        return -1;
    }

    return *id;
}

// Forward lookup (get token by array)
char* mistral_get_token_by_id(TokenizerModel* tokenizer, int32_t id) {
    if (!tokenizer || id < 0 || id >= tokenizer->vocab_size) {
        LOG_ERROR("%s: Invalid arguments or ID out of range.\n", __func__);
        return NULL;
    }

    Token* token = tokenizer->tokens[id];
    if (!token) {
        LOG_WARN("%s: Token with ID %d not found.\n", __func__, id);
        return NULL;
    }

    return token->data;
}

MistralModel* mistral_read_model(char* model_path) {
    MistralModel* mistral_model = (MistralModel*) malloc(sizeof(MistralModel));
    if (!mistral_model) {
        return NULL;
    }

    // Open the model file
    MagicFile* magic_file = magic_file_open(model_path, "rb");
    if (!magic_file) {
        LOG_ERROR("Failed to open model file: %s", model_path);
        return NULL;
    }
    // Validate the model file
    if (MAGIC_SUCCESS != magic_file_validate(magic_file)) {
        LOG_ERROR("Invalid model file: %s", model_path);
        magic_file_close(magic_file);
        return NULL;
    }

    // Read the models magic header (start section)
    mistral_model->magic = mistral_read_start_section(magic_file);
    if (!mistral_model->magic) {
        magic_file_close(magic_file);
        return NULL;
    }

    // Read the models general section
    mistral_model->general = mistral_read_general_section(magic_file);
    if (!mistral_model->general) {
        mistral_free_model(mistral_model);
        magic_file_close(magic_file);
        return NULL;
    }
    mistral_log_general_section(mistral_model->general);

    // Read the models parameters section
    mistral_model->parameters = mistral_read_parameters_section(magic_file);
    if (!mistral_model->parameters) {
        mistral_free_model(mistral_model);
        magic_file_close(magic_file);
        return NULL;
    }
    mistral_log_parameters_section(mistral_model->parameters);

    mistral_model->tokenizer = mistral_read_tokenizer_section(magic_file);
    if (!mistral_model->tokenizer) {
        mistral_free_model(mistral_model);
        magic_file_close(magic_file);
        return NULL;
    }
    mistral_log_tokenizer_section(mistral_model->tokenizer);

    // Close the model file
    if (MAGIC_SUCCESS != magic_file_close(magic_file)) {
        LOG_ERROR("%s: Failed to close model file: %s", __func__, magic_file->filepath);
        mistral_free_model(mistral_model);
        return NULL;
    }

    return mistral_model;
}

void mistral_free_model(MistralModel* mistral_model) {
    if (mistral_model) {
        mistral_free_tokenizer_section(mistral_model->tokenizer);
        mistral_free_parameters_section(mistral_model->parameters);
        mistral_free_general_section(mistral_model->general);
        mistral_free_start_section(mistral_model->magic);
        free(mistral_model);
    }
}
