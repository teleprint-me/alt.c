/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/interface/flex_string.h
 * @brief Interface for flexible string manipulation supporting common ASCII and UTF-8 operations.
 */

#ifndef ALT_FLEX_STRING_H
#define ALT_FLEX_STRING_H

// Must be defined before including pcre2.h
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

#include <stdlib.h>
#include <string.h>

#include "interface/logger.h"

// Macros

/**
 * @brief Validates the input string and key for null or empty values.
 *
 * @param input The input string to check.
 * @param key A pointer to a character or string, such as a delimiter or pattern.
 *
 * If either the input or key is NULL or empty, an error is logged.
 */
#define FLEX_STRING_GUARD(input, key) \
    if (!(input) || *(input) == '\0' || !(key)) { \
        LOG_ERROR("%s: Invalid input: input or key is NULL\n", __func__); \
        return NULL; \
    }

// Structures

/**
 * @brief Structure representing a flexible string with multiple parts.
 */
typedef struct FlexString {
    char** parts; ///< Array of split strings
    uint32_t length; ///< Number of parts (strings) in the array
} FlexString;

// Flex Life-cycle

/**
 * @brief Creates and initializes a default FlexString object.
 *
 * The object is initialized to null and length 0.
 *
 * @return A pointer to the newly created FlexString object.
 */
FlexString* flex_string_create(void);

/**
 * @brief Frees the memory used by a FlexString object.
 *
 * @param flex_string A pointer to the FlexString object to free.
 */
void flex_string_free(FlexString* flex_string);

// Flex Operations

/**
 * @brief Splits a string into parts based on a delimiter, similar to Python's str.split().
 *
 * @param input The input string to split.
 * @param delimiter The delimiter used to split the string.
 *
 * @return A pointer to a FlexString object containing the parts of the split string.
 */
FlexString* flex_string_create_split(const char* input, const char* delimiter);

/**
 * @brief Tokenizes the input string based on a PCRE2-compatible regular expression pattern.
 *
 * @param input The string to tokenize.
 * @param pattern The regular expression pattern to use for tokenization.
 *
 * @return A pointer to a FlexString object containing the tokens from the input string.
 */
FlexString* flex_string_create_tokens(const char* input, const char* pattern);

// String Operations

/// @note These should be multi-byte (UTF-8) compatible. They just need some minor adjustments.
/// instead of using char, we should pass char* to account for the possibility of more than one
/// byte. char made the initial prototypes easier to implement. But this won't be compatible in the
/// long term.

/**
 * @brief Substitutes all occurrences of a target character in the input string with a replacement
 * string.
 *
 * @param input The input string.
 * @param replacement The string to replace the target with.
 * @param target The character to replace in the input string.
 *
 * @return A new string with the substitutions applied.
 */
char* flex_string_substitute_char(const char* input, const char* replacement, char target);

/**
 * @brief Prepends a character to the input string.
 *
 * @param input The input string.
 * @param prepend The character to prepend to the string.
 *
 * @return A new string with the prepended character.
 */
char* flex_string_prepend_char(const char* input, char prepend);

/**
 * @brief Appends a character to the input string.
 *
 * @param input The input string.
 * @param append The character to append to the string.
 *
 * @return A new string with the appended character.
 */
char* flex_string_append_char(const char* input, char append);

/**
 * @brief Joins two strings into a new string.
 * 
 * @param a The first string.
 * @param b The second string.
 * 
 * @return A new string that contains the concatenation of `a` and `b`.
 */
char* flex_string_join(const char* a, const char* b);

#endif // ALT_FLEX_STRING_H
