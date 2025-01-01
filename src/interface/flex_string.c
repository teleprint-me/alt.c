/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/interface/flex_string.c
 * @brief Interface for flexible string manipulation supporting common ASCII and UTF-8 operations.
 */

#include <stdlib.h>
#include <string.h>

#include "interface/logger.h"

#include "interface/flex_string.h"

FlexString* flex_string_create_split(const char* input, const char* delimiter) {
    if (!input || !delimiter || *input == '\0') {
        return NULL;
    }

    FlexString* split = (FlexString*) malloc(sizeof(FlexString));
    if (!split) {
        return NULL;
    }
    split->length = 0;
    split->parts = NULL;

    char* temp = strdup(input);
    char* token = strtok(temp, delimiter);
    while (token) {
        split->parts = realloc(split->parts, (split->length + 1) * sizeof(char*));
        if (!split->parts) {
            free(temp);
            free(split);
            return NULL;
        }
        split->parts[split->length] = strdup(token);
        split->length += 1;
        token = strtok(NULL, delimiter);
    }

    free(temp);
    return split;
}

void flex_string_free_split(FlexString* split) {
    if (split) {
        for (uint32_t i = 0; i < split->length; ++i) {
            if (split->parts[i]) {
                free(split->parts[i]);
            }
        }
        if (split->parts) {
            free(split->parts);
        }
        free(split);
    }
}

FlexString* flex_string_tokenize(const char* input, const char* pattern) {
    if (!input || !pattern) {
        LOG_ERROR("%s: Invalid input: input, pattern, or token_count is NULL\n", __func__);
        return NULL;
    }

    pcre2_code* re;
    PCRE2_SIZE erroffset;
    int errorcode;
    PCRE2_UCHAR8 buffer[256];

    re = pcre2_compile(
        (PCRE2_SPTR) pattern,
        PCRE2_ZERO_TERMINATED,
        PCRE2_UTF | PCRE2_UCP,
        &errorcode,
        &erroffset,
        NULL
    );

    if (!re) {
        pcre2_get_error_message(errorcode, buffer, sizeof(buffer));
        LOG_ERROR("%s: PCRE2 compilation failed at offset %zu: %s\n", __func__, erroffset, buffer);
        return NULL;
    }

    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(re, NULL);
    if (!match_data) {
        LOG_ERROR("%s: Failed to create match data\n", __func__);
        pcre2_code_free(re);
        return NULL;
    }

    char** tokens = NULL;
    *token_count = 0;

    const char* cursor = input;
    size_t subject_length = strlen(input);

    while (subject_length > 0) {
        int rc = pcre2_match(re, (PCRE2_SPTR) cursor, subject_length, 0, 0, match_data, NULL);
        if (rc <= 0) {
            break;
        }

        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
        size_t match_length = ovector[1] - ovector[0];

        char* token = strndup(cursor + ovector[0], match_length);
        if (!token) {
            break;
        }

        char** temp = realloc(tokens, sizeof(char*) * (*token_count + 2));
        if (!temp) {
            free(token);
            break;
        }
        tokens = temp;
        tokens[(*token_count)++] = token;
        tokens[*token_count] = NULL;

        cursor += ovector[1];
        subject_length -= ovector[1];
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    return tokens;
}

char* flex_string_substitute(
    const char* source_string, const char* replacement_string, char target_char
) {
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
