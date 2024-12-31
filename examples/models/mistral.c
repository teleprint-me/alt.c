/**
 * @file examples/models/mistral.c
 *
 * @brief Example program showcasing how the magic model file interface operates.
 *
 * @note This example only loads the tokenizer and excludes the weights for the mistral model.
 *
 * @todo Add implementation for handling weights and biases.
 *
 * Currently working on the tokenizer pipeline. The tokenizer requires multiple steps for processing
 * encoding and decoding inputs and outputs.
 *
 * - Add the GPT-2 Pre Tokenizer Regular Expression
 * - Add Pre Tokenization and return an array of strings
 * - Replace spaces (' ') with the meta-character (▁) as needed for alignment with SentencePiece.
 * - Decompose tokens into BPE compatible UTF-8 sequences
 */

// must be defined before including pcre2.h
#define PCRE2_CODE_UNIT_WIDTH 8

#include <locale.h>
#include <pcre2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// interfaces
#include "interface/logger.h"
#include "interface/path.h" // similar to how python os, os.path, and pathlib operate

// models
#include "model/magic.h"
#include "model/mistral.h"

#define GPT_PRE_TOKENIZER_REGEX \
    "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+(?!\\S)|\\s+)"

char** mistral_pre_tokenize(const char* input) {
    pcre2_code* re;
    PCRE2_SIZE erroffset;
    int errorcode;
    PCRE2_UCHAR8 buffer[256];

    re = pcre2_compile(
        (PCRE2_SPTR) GPT_PRE_TOKENIZER_REGEX,
        PCRE2_ZERO_TERMINATED,
        PCRE2_UTF | PCRE2_UCP, // UTF-8 and Unicode properties
        &errorcode,
        &erroffset,
        NULL
    );

    if (!re) {
        pcre2_get_error_message(errorcode, buffer, sizeof(buffer));
        fprintf(stderr, "PCRE2 compilation failed at offset %zu: %s\n", erroffset, buffer);
        return NULL;
    }

    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(re, NULL);

    const char* cursor = input;
    size_t subject_length = strlen(input);

    // Dynamic array to hold tokens
    char** tokens = NULL;
    size_t token_count = 0;

    while (*cursor) {
        int rc = pcre2_match(re, (PCRE2_SPTR) cursor, subject_length, 0, 0, match_data, NULL);

        if (rc > 0) {
            PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);

            // Process the primary match (ovector[0] and ovector[1])
            PCRE2_SPTR start = (PCRE2_SPTR) cursor + ovector[0];
            PCRE2_SIZE length = ovector[1] - ovector[0];

            char* token = strndup((const char*) start, length);
            if (!token) {
                fprintf(stderr, "Memory allocation failed for token.\n");
                break;
            }

            // Add token to the dynamic array
            tokens = realloc(tokens, sizeof(char*) * (token_count + 2));
            if (!tokens) {
                fprintf(stderr, "Memory allocation failed for token array.\n");
                free(token);
                break;
            }
            tokens[token_count++] = token;
            tokens[token_count] = NULL; // NULL-terminate the array

            // Advance cursor past the current match
            cursor += ovector[1];
            subject_length -= ovector[1];
        } else {
            break; // No more matches
        }
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    return tokens;
}

void mistral_tokenize(TokenizerModel* tokenizer, const char* input) {
    // Get user input as pre-tokenized array
    char** tokens = mistral_pre_tokenize(input);

    for (size_t i = 0; tokens[i] != NULL; i++) {
        char* token = tokens[i]; // Current token
        size_t token_len = strlen(token);

        // Allocate a buffer for the substituted token
        size_t buffer_len = token_len * 3 + 1; // Worst case: every char is replaced by '▁' (3 bytes)
        char* processed_token = malloc(buffer_len);
        if (!processed_token) {
            fprintf(stderr, "Memory allocation failed for processed_token.\n");
            continue;
        }

        size_t buffer_index = 0;
        for (size_t j = 0; j < token_len; j++) {
            if (token[j] == ' ') {
                // Replace ' ' with UTF-8 encoding of '▁'
                const char* marker = "\xe2\x96\x81"; // UTF-8 encoding for '▁'
                size_t marker_len = strlen(marker);

                if (buffer_index + marker_len < buffer_len) {
                    memcpy(processed_token + buffer_index, marker, marker_len);
                    buffer_index += marker_len;
                }
            } else {
                // Copy the character as is
                if (buffer_index + 1 < buffer_len) {
                    processed_token[buffer_index++] = token[j];
                }
            }
        }

        processed_token[buffer_index] = '\0'; // Null-terminate the processed token

        // Attempt to get the ID of the token
        int32_t token_id = mistral_get_id_by_token(tokenizer, processed_token);
        if (token_id == -1) {
            token_id = tokenizer->unk_id; // Handle unknown token
        }

        // Print the results
        printf("Token: '%s', ID: %d\n", processed_token, token_id);

        // Free the processed token buffer
        free(processed_token);
    }

    // Free the tokens array
    for (size_t i = 0; tokens[i] != NULL; i++) {
        free(tokens[i]);
    }
    free(tokens);
}

int main(int argc, char* argv[]) {
    global_logger.log_level = LOG_LEVEL_INFO;
    setlocale(LC_ALL, "en_US.UTF-8");

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <model_file> <input>\n", argv[0]);
        return 1;
    }

    char* model_path = argv[1];
    char* user_input = NULL;
    size_t input_size = 0;

    // Concatenate all input arguments into a single string
    for (int i = 2; i < argc; i++) {
        size_t arg_len = strlen(argv[i]);
        size_t new_size = input_size + arg_len + 2; // +1 for space or null-terminator

        // Allocate (or reallocate) memory for the concatenated string
        char* temp = realloc(user_input, new_size);
        if (!temp) {
            fprintf(stderr, "Memory allocation failed.\n");
            free(user_input);
            return 1;
        }

        user_input = temp;

        // Append the argument and a space (or null-terminate at the end)
        strcpy(user_input + input_size, argv[i]);
        input_size += arg_len;

        if (i < argc - 1) {
            user_input[input_size++] = ' ';
        } else {
            user_input[input_size] = '\0';
        }
    }

    printf("Model Path: %s\n", model_path);
    printf("User Input: %s\n", user_input);

    MistralModel* mistral_model = mistral_read_model(model_path);
    if (!mistral_model) {
        return EXIT_FAILURE;
    }

    mistral_tokenize(mistral_model->tokenizer, user_input);

    // Cleanup
    free(user_input);
    mistral_free_model(mistral_model);

    return EXIT_SUCCESS;
}
