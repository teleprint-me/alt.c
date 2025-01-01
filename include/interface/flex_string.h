/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/interface/flex_string.h
 * @brief Interface for flexible string manipulation supporting common ASCII and UTF-8 operations.
 */

#ifndef ALT_FLEX_STRING_H
#define ALT_FLEX_STRING_H

// must be defined before including pcre2.h
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

// Include clib string as defacto
#include <string.h>

#include "interface/logger.h"

typedef struct FlexString {
    char** parts;
    uint32_t length;
} FlexString;

/**
 * @param input The input string.
 * @param key A pointer to a character or string.
 * @note The key maybe a delimeter, pattern, etc.
 */
#define FLEX_STRING_GUARD(input, key) \
    if (!(input) || !(key) || *(input) == '\0') { \
        LOG_ERROR("%s: Invalid input: input, pattern, or token_count is NULL\n", __func__); \
        return NULL; \
    }

// Create the default object (set to null and 0)
FlexString* flex_string_create(void);
// Function for freeing flex string objects
void flex_string_free(FlexString* flex_string);

// Behaves somewhat like Pythons str.split()
FlexString* flex_string_create_split(const char* input, const char* delimeter);
// Tokenize a given source string using a PCRE2 compatible expression.
FlexString* flex_string_create_tokens(const char* input, const char* pattern);

// Substitute occurrences of target in input with replacement
char* flex_string_substitute(const char* input, const char* replacement, char target);

#endif // ALT_FLEX_STRING_H
