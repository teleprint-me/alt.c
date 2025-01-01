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

typedef struct FlexString {
    char** parts;
    uint32_t length;
} FlexString;

// Behaves somewhat like Pythons str.split()
FlexString* flex_string_create_split(const char* input, const char* delimeter);
void flex_string_free_split(FlexString* string_split);

// Tokenize a given source string using a PCRE2 compatible expression.
FlexString* flex_string_create_tokens(const char* input, const char* pattern);
void flex_string_free_tokens(FlexString* string_tokens);

// Substitute occurrences of target in input with replacement
char* flex_string_substitute(const char* input, const char* replacement, char target);

#endif // ALT_FLEX_STRING_H
