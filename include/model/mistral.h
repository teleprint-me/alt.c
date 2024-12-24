/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/model/mistral.h
 *
 * See specification for more details about the Altiera model file format.
 *
 * @ref docs/model/specification.md
 */

#ifndef ALT_MODEL_MISTRAL_H
#define ALT_MODEL_MISTRAL_H

#include <stdbool.h>

#include "model/file.h"

typedef struct String {
    uint32_t length;
    char* data;
} String;

typedef struct MagicModel {
    int32_t version;
    int32_t alignment;
} MagicModel;

typedef struct GeneralModel {
    String model_type;
    String model_base;
    String author;
    String created_at;
    String last_modified;
    String license;
    String uuid;
} GeneralModel;

typedef struct ParametersModel {
    String hidden_act;
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
} ParametersModel;

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
    TokenType type;
    int32_t length;
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
/// @note Tensors are a currently a work in progress.

typedef struct MistralModel {
    MagicModel* magic;
    GeneralModel* general;
    ParametersModel* parameters;
    TokenizerModel* tokenizer;
} MistralModel;

#endif // ALT_MODEL_MISTRAL_H
