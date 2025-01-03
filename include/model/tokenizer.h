/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/model/tokenizer.h
 * @brief Interface for string and token manipulation supporting common ASCII and UTF-8 operations.
 * @note The tokenizer types are implementation specific. This API simply provides them. Model
 * implementation details are left up to the user. The models are converted from SentencePiece
 * binary format to ALT binary format. The tokenizer types here follow the expected file format
 * according to the specification.
 * @warning The implementation should expect BPE or BPE-like models and does not support unigram.
 *
 * This module is responsible for converting and handling user inputs and outputs.
 * - Normalization (NFKC Unicode)
 * - Pre-tokenization
 * - Merge pairs
 * - Merge frequencies
 * - Substitutions
 * - Post-processing
 * - Training
 * - Calculating frequency and score probabilities
 * - Token to id conversion
 * - Id to token conversion
 */

#ifndef ALT_TOKENIZER_H
#define ALT_TOKENIZER_H

// must be defined before including pcre2.h
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

// Standard C libraries
#include <string.h>

// Algorithms
#include "algorithm/hash.h"

// Interfaces
#include "interface/flex_string.h"

// ---------------------- Macros ----------------------

// Googles meta marker representing a space
#define TOKEN_META_MARKER "\u2581" // UTF-8 marker '▁'

// OpenAI's pre tokenizer perl5 compatible pattern. GPT is good enough for most cases.
#define GPT_PRE_TOKENIZER_REGEX \
    "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+(?!\\S)|\\s+)"

// Add more pre tokenization patterns as needed.

// ---------------------- MagicFile structures ----------------------

/// @note These are model independent, but are designed to mirror the SentencePiece Processor to
/// enable compatibility with state-of-the-art methods. For these reasons, the implementations are
/// left up to the end user. See MagicFile API for more information.

typedef enum TokenType {
    TOKEN_NORMAL,
    TOKEN_UNKNOWN,
    TOKEN_CONTROL,
    TOKEN_USER_DEFINED,
    TOKEN_BYTE,
    TOKEN_UNUSED,
    TOKEN_BOS,
    TOKEN_EOS,
    TOKEN_PAD,
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

// ---------------------- Byte-pair structures ----------------------

typedef struct VocabularyEntry {
    char* word; // Space-separated symbols
    int* frequency; // Pointer to frequency count
} VocabularyEntry;

// ---------------------- Byte-pair operations ----------------------

VocabularyEntry* create_vocab_entry(const char* word, int frequency);
void free_vocab_entry(VocabularyEntry* entry);

HashTable* get_stats(HashTable* vocab);
void free_stats(HashTable* stats);

void merge_vocab(HashTable* vocab, const char* pair);

// The TOKEN_BYTE flag can be used to determine if the token is a byte representation or not
TokenType get_token_type(Token* token);

// ---------------------- Byte-pair mappings ----------------------

/**
 * @brief Initializes a hash table for token-to-byte mapping.
 *
 * @return A pointer to the initialized hash table.
 */
HashTable* create_byte_map(void);

/**
 * @brief Frees a hash table from token-to-byte mapping.
 */
void free_byte_map(HashTable* byte_map);

/**
 * @brief Converts a byte value to a token string representation.
 *
 * @param byte The byte value to convert.
 * @return A dynamically allocated string representing the byte in hex format (e.g., "<0x01>").
 */
char* byte_to_token(unsigned char byte);

/**
 * @brief Converts a token string back to its corresponding byte value.
 *
 * @param map The hash table containing the token-to-byte mappings.
 * @param token The token string to search for.
 * @return The byte corresponding to the token, or -1 if not found.
 */
int token_to_byte(HashTable* byte_map, const char* token);

#endif // ALT_TOKENIZER_H
