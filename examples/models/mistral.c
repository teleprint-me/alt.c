/**
 * @file examples/models/mistral.c
 *
 * @brief Example program showcasing how the magic model file interface operates.
 *
 * @note This example only loads the tokenizer and excludes the weights for the mistral model.
 *
 * @todo Add implementation for handling weights and biases.
 */

#include <locale.h>
#include <stdio.h>

// interfaces
#include "interface/logger.h"
#include "interface/path.h" // similar to how python os, os.path, and pathlib operate

// models
#include "model/magic.h"
#include "model/mistral.h"

/// @note Not sure how to handle this yet. Still figuring it out.
/// Mistral uses BPE, but we can start off with a naive representation.
void mistral_tokenize(TokenizerModel* tokenizer, const char* input) {
    char* token = strtok(strdup(input), " .,!?;:\"()");
    while (token) {
        int32_t id = mistral_get_id_by_token(tokenizer, token);
        if (id == -1) {
            id = tokenizer->unk_id; // Handle unknown token
        }
        printf("Token: %s, ID: %d\n", token, id);
        token = strtok(NULL, " .,!?;:\"()");
    }
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
    if (!mistral_model) { return EXIT_FAILURE; }

    mistral_tokenize(mistral_model->tokenizer, user_input);

    // Cleanup
    free(user_input);
    mistral_free_model(mistral_model);

    return EXIT_SUCCESS;
}
