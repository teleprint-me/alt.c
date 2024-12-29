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
void tokenize_utf8(const char* text) {
    const char* delimiters = " .,!?;:\"()";
    char* text_copy = strdup(text); // Make a mutable copy
    char* token = strtok(text_copy, delimiters);

    while (token) {
        printf("Token: %s\n", token);
        token = strtok(NULL, delimiters);
    }

    free(text_copy);
}

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "en_US.UTF-8");

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <model_file>\n", argv[0]);
        return 1;
    }
    char* model_path = argv[1];

    // Open the model file
    MagicFile* magic_file = magic_file_open(model_path, "rb");
    if (!magic_file) {
        LOG_ERROR("Failed to open model file: %s", model_path);
        return MAGIC_FILE_ERROR;
    }

    // Validate the model file
    if (MAGIC_SUCCESS != magic_file_validate(magic_file)) {
        LOG_ERROR("Invalid model file: %s", model_path);
        magic_file_close(magic_file);
        return MAGIC_FILE_ERROR;
    }

    // Read the models magic header (start section)
    MistralMagic* header = mistral_read_start_section(magic_file);
    if (!header) {
        magic_file_close(magic_file);
        return MAGIC_ERROR;
    }

    // Read the models general section
    MistralGeneral* general = mistral_read_general_section(magic_file);
    if (!general) {
        mistral_free_start_section(header);
        magic_file_close(magic_file);
        return MAGIC_ERROR;
    }
    mistral_log_general_section(general);

    // Read the models parameters section
    MistralParameters* parameters = mistral_read_parameters_section(magic_file);
    if (!parameters) {
        mistral_free_general_section(general);
        mistral_free_start_section(header);
        magic_file_close(magic_file);
        return MAGIC_ERROR;
    }
    mistral_log_parameters_section(parameters);

    TokenizerModel* tokenizer = mistral_read_tokenizer_section(magic_file);
    if (!tokenizer) {
        mistral_free_parameters_section(parameters);
        mistral_free_general_section(general);
        mistral_free_start_section(header);
        magic_file_close(magic_file);
        return MAGIC_ERROR;
    }
    mistral_log_tokenizer_section(tokenizer);

    // Close the model file
    if (MAGIC_SUCCESS != magic_file_close(magic_file)) {
        LOG_ERROR("%s: Failed to close model file: %s", __func__, magic_file->filepath);
        return MAGIC_FILE_ERROR;
    }

    // Cleanup
    mistral_free_tokenizer_section(tokenizer);
    mistral_free_parameters_section(parameters);
    mistral_free_general_section(general);
    mistral_free_start_section(header);
    magic_file_close(magic_file);

    return MAGIC_SUCCESS;
}
