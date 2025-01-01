/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/interface/flex_string.h
 * @brief Interface for flexible string manipulation supporting common ASCII and UTF-8 operations.
 */

#ifndef ALT_FLEX_STRING_H
#define ALT_FLEX_STRING_H

// Include clib string as defacto
#include <string.h>

// Substitute occurrences of target_char in source_string with replacement_string
char* flex_string_sub_char_with_uft8(
    const char* source_string, const char* replacement_string, char target_char
);

#endif // ALT_FLEX_STRING_H
