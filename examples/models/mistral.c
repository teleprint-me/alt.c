/**
 * @file examples/models/mistral.c
 *
 * @brief Example program showcasing how the magic model file interface operates.
 *
 * @note This example only loads the tokenizer and excludes the weights for the mistral model.
 *
 * @todo Add implementation for handling weights and biases.
 */

#include <stdio.h>

#include "logger.h"
#include "path.h" // similar to how python os, os.path, and pathlib operate
#include "model/magic.h"
#include "model/mistral.h"

int main(int argc, char* argv[]) {
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
    MistralMagic* mistral_magic = mistral_read_start_section(magic_file);

    // Read the models general section
    MistralGeneral* mistral_general = mistral_read_general_section(magic_file);
    mistral_log_general_section(mistral_general);

    // MistralParameters parameters = {0};

    if (MAGIC_SUCCESS != magic_file_close(magic_file)) {
        LOG_ERROR("%s: Failed to close model file: %s", __func__, magic_file->filepath);
        return MAGIC_FILE_ERROR;
    }

    // Cleanup
    mistral_free_general_section(mistral_general);
    mistral_free_start_section(mistral_magic);

    return MAGIC_SUCCESS;
}