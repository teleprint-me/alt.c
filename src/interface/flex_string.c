/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/interface/flex_string.c
 * @brief Interface for flexible string manipulation supporting common ASCII and UTF-8 operations.
 */

#include "interface/logger.h"
#include "interface/flex_string.h"

// Substitute occurrences of target_char in source_string with replacement_string
char* string_replace_char_with_uft8(const char* source_string, const char* replacement_string, char target_char) {
    if (!source_string || !replacement_string) {
        LOG_ERROR("%s: source_string or replacement_string is NULL\n", __func__);
        return NULL;
    }

    size_t source_length = strlen(source_string);
    size_t replacement_length = strlen(replacement_string);
    size_t target_count = 0;

    // Count occurrences of target_char in source_string
    for (size_t i = 0; i < source_length; i++) {
        if (source_string[i] == target_char) {
            target_count++;
        }
    }

    // Calculate the new string's length (+1 for null terminator)
    size_t result_length = source_length + target_count * (replacement_length - 1) + 1;
    char* result = (char*) malloc(result_length);
    if (!result) {
        LOG_ERROR("%s: Failed to allocate memory\n", __func__);
        return NULL;
    }

    // Build the resulting string
    const char* src = source_string;
    char* dest = result;
    while (*src) {
        if (*src == target_char) {
            strcpy(dest, replacement_string);
            dest += replacement_length;
        } else {
            *dest++ = *src;
        }
        src++;
    }
    *dest = '\0';

    return result;
}
