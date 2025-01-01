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

// Substitute occurrences of target_char in source_string with replacement_string
char* flex_string_substitute(
    const char* source_string, const char* replacement_string, char target_char
);

// Tokenize a given source string using a PCRE2 compatible expression.
char** flex_string_tokenize(const char* input, const char* pattern, size_t* token_count) ;

#endif // ALT_FLEX_STRING_H
