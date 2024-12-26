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

typedef struct Token {
    float score;
    int32_t length;
    TokenType type;
    char* piece;
} Token;

typedef struct TokenizerModel {
    int32_t vocab_size;
    int32_t bos_id;
    int32_t eos_id;
    int32_t pad_id;
    int32_t unk_id;
    int32_t seq_len;
    Token** tokens;
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

MistralMagic* mistral_read_start_section(MagicFile* magic_file);
void mistral_free_start_section(MistralMagic* mistral_magic);

MistralGeneral* mistral_read_general_section(MagicFile* magic_file);
void mistral_free_general_section(MistralGeneral* mistral_general);
void mistral_log_general_section(MistralGeneral* mistral_general);

#endif // ALT_MODEL_MISTRAL_H
