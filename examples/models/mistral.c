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

    // Read the models magic header
    MistralMagic* mistral_magic = mistral_read_start_marker(magic_file);

    // Read the models general section
    MistralGeneral* mistral_general = (MistralGeneral*) malloc(sizeof(MistralGeneral));
    if (!mistral_general) {
        LOG_ERROR("%s: Failed to allocate memory to MistralGeneral.\n", __func__);
        return MAGIC_ERROR;
    }
    int64_t general_marker = 0;
    int64_t general_size = 0;
    magic_file_read_section_marker(magic_file, &general_marker, &general_size);
    // for each element in the general section, there is a string length follow by the string data.
    
    if (MAGIC_SUCCESS != magic_file_read_string_field(magic_file, &mistral_general->model_type)) {
        LOG_ERROR("Failed to read model_type from general section.");
        return MAGIC_FILE_ERROR;
    }
    LOG_INFO("%s: general->model_type=%s\n", __func__, mistral_general->model_type);
    // MistralParameters parameters = {0};

    if (MAGIC_SUCCESS != magic_file_close(magic_file)) {
        LOG_ERROR("%s: Failed to close model file: %s", __func__, magic_file->filepath);
        return MAGIC_FILE_ERROR;
    }

    // Cleanup
    free(mistral_magic);
    free(mistral_general->model_type);
    free(mistral_general);

    return MAGIC_SUCCESS;
}
