/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/interface/flex_string.c
 * @brief Interface for flexible string manipulation supporting common ASCII and UTF-8 operations.
 */

#include <stdlib.h>
#include <string.h>

#include "interface/flex_string.h"

// Create a default object for populating
FlexString* flex_string_create(void) {
    FlexString* flex_string = (FlexString*) malloc(sizeof(FlexString));
    if (!flex_string) {
        return NULL;
    }
    flex_string->length = 0;
    flex_string->parts = NULL;
    return flex_string;
}

void flex_string_free(FlexString* flex_string) {
    if (flex_string) {
        for (uint32_t i = 0; i < flex_string->length; ++i) {
            if (flex_string->parts[i]) {
                free(flex_string->parts[i]);
            }
        }
        if (flex_string->parts) {
            free(flex_string->parts);
        }
        free(flex_string);
    }
}

FlexString* flex_string_create_split(const char* input, const char* delimiter) {
    FLEX_STRING_GUARD(input, delimiter);

    FlexString* split = flex_string_create();

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

FlexString* flex_string_create_tokens(const char* input, const char* pattern) {
    FLEX_STRING_GUARD(input, pattern);

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

    const char* cursor = input;
    size_t subject_length = strlen(input);
    FlexString* token = flex_string_create();
    if (!token) {
        pcre2_match_data_free(match_data);
        pcre2_code_free(re);
        return NULL;
    }

    while (subject_length > 0) {
        int rc = pcre2_match(re, (PCRE2_SPTR) cursor, subject_length, 0, 0, match_data, NULL);
        if (rc <= 0) {
            break;
        }

        PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
        size_t match_length = ovector[1] - ovector[0];

        char* part = strndup(cursor + ovector[0], match_length);
        if (!part) {
            break;
        }

        char** temp = realloc(token->parts, sizeof(char*) * (token->length + 1));
        if (!temp) {
            free(part);
            break;
        }
        token->parts = temp;
        token->parts[token->length++] = part;

        cursor += ovector[1];
        subject_length -= ovector[1];
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    return token;
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
