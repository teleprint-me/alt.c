/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/model/mistral.h
 *
 * See specification for more details about the Altiera model file format.
 * @ref docs/model/specification.md
 *
 * @note Mistrals tokenizer is a sentencepiece byte-pair encoding model. Unsure of how I'd like to
 * handle this at the moment. Will figure it out later.
 */

#ifndef ALT_MODEL_MISTRAL_H
#define ALT_MODEL_MISTRAL_H

#include <stdbool.h>

#include "algorithm/hash_table.h"

#include "model/magic.h"

// ------------------------ Model structures ------------------------

typedef struct MistralMagic {
    int32_t version;
    int32_t alignment;
} MistralMagic;

typedef struct MistralGeneral {
    char* model_type;
    char* model_base;
    char* author;
    char* created_at;
    char* last_modified;
    char* license;
    char* uuid;
} MistralGeneral;

typedef struct MistralParameters {
    bool tie_word_embeddings;
    int32_t hidden_size;
    int32_t intermediate_size;
    int32_t max_position_embeddings;
    int32_t num_attention_heads;
    int32_t num_hidden_layers;
    int32_t num_key_value_heads;
    int32_t sliding_window;
    int32_t head_size;
    float rope_theta;
    float rms_norm_eps;
    float initializer_range;
    char* hidden_act;
} MistralParameters;

typedef enum TokenType {
    NORMAL = 0,
    BYTE = 1,
    CONTROL = 2,
    UNKNOWN = 3,
    UNUSED = 4,
    BOS = 5,
    EOS = 6,
    PAD = 7,
} TokenType;

typedef struct __attribute__((aligned(8))) Token {
    float score; // Log-probability of the token data. Use exp(score) to get actual frequency.
    int32_t type; // Type of the token (e.g., NORMAL, BOS, EOS)
    int32_t id; // Encoded ID representing the position (e.g. 0, 1, 2)
    int32_t length; // Length of the token string
    char* data; // UTF-8 encoded string (dynamically allocated)
} Token;

typedef struct __attribute__((aligned(8))) TokenizerModel {
    int32_t vocab_size; // Total number of tokens
    int32_t bos_id; // Beginning-of-sequence token ID
    int32_t eos_id; // End-of-sequence token ID
    int32_t pad_id; // Padding token ID
    int32_t unk_id; // Unknown token ID
    Token** tokens; // Array of tokens, indexed by ID
    HashTable* table; // Hash map for string-based lookups
} TokenizerModel;

/// @todo TensorsModel
/// @note Tensors are a currently a work in progress. Depends upon specification details and python
/// implementation.

typedef struct MistralModel {
    MistralMagic* magic;
    MistralGeneral* general;
    MistralParameters* parameters;
    TokenizerModel* tokenizer;
    // tensors will go here once implemented
} MistralModel;

// ------------------------ Model file functions ------------------------

// Read the magic header
MistralMagic* mistral_read_start_section(MagicFile* magic_file);
void mistral_free_start_section(MistralMagic* header);

// Read the general section
MistralGeneral* mistral_read_general_section(MagicFile* magic_file);
void mistral_free_general_section(MistralGeneral* general);
void mistral_log_general_section(MistralGeneral* general);

// Read the parameters section
MistralParameters* mistral_read_parameters_section(MagicFile* magic_file);
void mistral_free_parameters_section(MistralParameters* parameters);
void mistral_log_parameters_section(MistralParameters* parameters);

// Read the tokenizer section
Token* mistral_read_token(MagicFile* magic_file);
void mistral_free_token(Token* token);
TokenizerModel* mistral_read_tokenizer_section(MagicFile* magic_file);
void mistral_free_tokenizer_section(TokenizerModel* tokenizer);
void mistral_log_tokenizer_section(TokenizerModel* tokenizer);

// Token lookup
int32_t mistral_get_id_by_token(TokenizerModel* tokenizer, const char* data);
char* mistral_get_token_by_id(TokenizerModel* tokenizer, int32_t id);

MistralModel* mistral_read_model(char* model_path);
void mistral_free_model(MistralModel* mistral_model);

#endif // ALT_MODEL_MISTRAL_H
