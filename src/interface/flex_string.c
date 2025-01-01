/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/interface/flex_string.c
 * @brief Interface for flexible string manipulation supporting common ASCII and UTF-8 operations.
 */

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

char* flex_string_substitute_char(const char* input, const char* replacement, char target) {
    FLEX_STRING_GUARD(input, replacement);

    size_t replacement_length = strlen(replacement);

    // Calculate new string length and build result in one pass
    size_t result_length = 0;
    const char* src = input;
    while (*src) {
        result_length += (*src == target) ? replacement_length : 1;
        src++;
    }

    char* result = (char*) malloc(result_length + 1);
    if (!result) {
        LOG_ERROR("%s: Failed to allocate memory\n", __func__);
        return NULL;
    }

    // Build the resulting string
    const char* source_cursor = input;
    char* result_cursor = result;
    while (*source_cursor) {
        if (*source_cursor == target) {
            strcpy(result_cursor, replacement);
            result_cursor += replacement_length;
        } else {
            *result_cursor++ = *source_cursor;
        }
        source_cursor++;
    }
    *result_cursor = '\0';

    return result;
}

// Function to prepend a character
char* flex_string_prepend_char(const char* input, char prepend) {
    size_t input_len = strlen(input);
    char* result = (char*) malloc(input_len + 2); // +2 for the prepend char and null terminator
    if (!result) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    result[0] = prepend;
    strcpy(result + 1, input);
    return result;
}

// Function to append a character
char* flex_string_append_char(const char* input, char append) {
    size_t input_len = strlen(input);
    char* result = (char*) malloc(input_len + 2); // +2 for the append char and null terminator
    if (!result) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    strcpy(result, input);
    result[input_len] = append;
    result[input_len + 1] = '\0';
    return result;
}
