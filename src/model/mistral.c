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

#include "logger.h"
#include "model/magic.h"
#include "model/mistral.h"

MistralMagic* mistral_read_start_marker(MagicFile* magic_file) {
    // Allocate memory for start section
    MistralMagic* mistral_magic = (MistralMagic*) malloc(sizeof(MistralMagic));
    if (!mistral_magic) {
        LOG_ERROR("%s: Failed to allocate memory to MistralMagic.\n", __func__);
        return NULL;
    }

    // Set default values
    mistral_magic->version = MAGIC_VERSION;
    mistral_magic->alignment = MAGIC_ALIGNMENT;

    // Read the start section
    magic_file_read_start_marker(magic_file, &mistral_magic->version, &mistral_magic->alignment);
    return mistral_magic;
}
