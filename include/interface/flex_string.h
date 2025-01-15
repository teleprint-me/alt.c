/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/interface/flex_string.h
 * @brief Flexible String API for common ASCII and UTF-8 string manipulation.
 *
 * Provides utilities for working with UTF-8 strings, including splitting,
 * joining, substitution, and regex-based operations.
 */

#ifndef ALT_FLEX_STRING_H
#define ALT_FLEX_STRING_H

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

// Include regex library for pattern matching (PCRE2 required)
#define PCRE2_CODE_UNIT_WIDTH 8
#include <pcre2.h>

// ---------------------- Enumerations ----------------------

typedef enum FlexStringCompareResult {
    FLEX_STRING_COMPARE_INVALID = -2,
    FLEX_STRING_COMPARE_LESS = -1,
    FLEX_STRING_COMPARE_EQUAL = 0,
    FLEX_STRING_COMPARE_GREATER = 1
} FlexStringCompareResult;

// ---------------------- Structures ----------------------

/**
 * @brief Represents a mutable string with dynamic memory management.
 */
typedef struct __attribute__((packed)) FlexString {
    uint8_t valid_utf8; ///< Indicates whether the string is valid UTF-8 (1 = true, 0 = false).
    uint32_t capacity; ///< Capacity of the allocated buffer (in bytes).
    uint32_t length; ///< Length of the string (in characters, not bytes).
    char* data; ///< Pointer to the string data.
} FlexString;

/**
 * @brief Represents a flexible string with multiple parts.
 */
typedef struct __attribute__((packed)) FlexStringSplit {
    uint8_t valid_utf8; ///< Indicates whether the string is valid UTF-8 (1 = true, 0 = false).
    uint32_t capacity; ///< Capacity of the `parts` array.
    uint32_t length; ///< Number of parts (strings) in the array.
    char** parts; ///< Array of split strings.
} FlexStringSplit;

// ---------------------- Lifecycle Functions ----------------------

/**
 * @brief Creates a FlexString with the given data.
 *
 * @param data The initial string data.
 * @note The FlexString automatically determines the length of the string.
 * @return Pointer to a newly allocated FlexString.
 */
FlexString* flex_string_create(char* data);

/**
 * @brief Frees the memory used by a FlexString.
 *
 * @param string Pointer to the FlexString to free.
 */
void flex_string_free(FlexString* string);

/**
 * @brief Creates an empty FlexStringSplit object.
 *
 * @param initial_capacity The initial capacity of the FlexStringSplit object.
 * @return Pointer to a newly allocated FlexStringSplit object.
 */
FlexStringSplit* flex_string_create_split(uint32_t initial_capacity);

/**
 * @brief Frees the memory used by a FlexStringSplit.
 *
 * @param split Pointer to the FlexStringSplit to free.
 */
void flex_string_free_split(FlexStringSplit* split);

// ---------------------- Character Operations ----------------------

/**
 * @brief Determines the length of a UTF-8 character.
 *
 * This function takes a single byte as input and returns the length of the corresponding UTF-8
 * character. It uses the UTF-8 character encoding rules to determine the length.
 *
 * @param byte The input byte to be processed.
 * @return The length of the UTF-8 character represented by the input byte, or -1 if the byte is
 * invalid.
 */
int8_t flex_string_utf8_char_length(uint8_t byte);

/**
 * @brief Validates a UTF-8 character.
 *
 * This function takes a pointer to a UTF-8 character (represented as a sequence of bytes) and
 * checks its validity based on UTF-8 character encoding rules.
 *
 * @param string The input string to be validated.
 * @param char_length The length of the input character.
 * @return true if the character is valid, false otherwise.
 */
bool flex_string_utf8_char_validate(const uint8_t* string, int8_t char_length);

/**
 * @brief Callback function for UTF-8 character iterator.
 *
 * This function is called for each character in a UTF-8 string.
 *
 * @param char_start The start of the UTF-8 character.
 * @param char_length The length of the UTF-8 character.
 * @param context The context pointer passed to the callback function.
 * @return true if the character is valid, false otherwise.
 */
typedef void* (*FlexStringUTF8Iterator)(
    const uint8_t* char_start, int8_t char_length, void* context
);

/**
 * @brief Iterates over a UTF-8 string.
 *
 * This function takes a pointer to a UTF-8 string and a callback function that is called for each
 * character in the string.
 *
 * @param input The input string to be iterated.
 * @param callback The callback function to be called for each character in the string.
 * @param context The context pointer passed to the callback function.
 * @return true if the string was successfully iterated, false otherwise.
 */
void* flex_string_utf8_char_iterator(
    const char* input, FlexStringUTF8Iterator callback, void* context
);

// ---------------------- UTF-8 String Operations ----------------------

/**
 * @brief Validates an entire UTF-8 string.
 *
 * This function iterates through the input string and validates each UTF-8 character
 * using the UTF-8 character encoding rules.
 *
 * @param input The input string to be validated.
 * @return true if the string is valid UTF-8, false otherwise.
 */
bool flex_string_utf8_string_validate(const char* input);

/**
 * @brief Calculates the symbolic length of a UTF-8 string in characters.
 *
 * @param input The input string.
 * @return The number of symbolic UTF-8 characters in the string.
 */
int32_t flex_string_utf8_string_char_length(const char* input);

/**
 * @brief Calculates the literal length of a UTF-8 string in bytes.
 *
 * @param input The input string.
 * @return The number of literal UTF-8 bytes in the string.
 */
int32_t flex_string_utf8_string_byte_length(const char* input);

/**
 * @brief Compares two UTF-8 strings.
 *
 * @param first The first string to compare.
 * @param second The second string to compare.
 * @return An integer indicating the result of the comparison:
 * -2 if the strings are not valid UTF-8 strings.
 * -1 if the first string is less than the second.
 * 0 if the strings are equal.
 * 1 if the first string is greater than the second.
 */
int32_t flex_string_utf8_string_compare(const char* first, const char* second);

/**
 * @brief Copies a UTF-8 string using a loop.
 *
 * This function copies a UTF-8 string using a loop to ensure precision,
 * avoiding potential issues with string length calculations.
 *
 * @param input The string to copy.
 * @note The caller is responsible for freeing the returned string.
 * @return The destination string, or NULL on failure.
 */
char* flex_string_utf8_string_copy(const char* input);

/**
 * @brief Concatenates the target string to the source string.
 *
 * @param source The string to concatenate to the target string.
 * @param target The string to concatenate to the source string.
 * @note The caller is responsible for freeing the returned string.
 * @return The concatenated string, or NULL on failure.
 */
char* flex_string_utf8_string_concat(const char* source, const char* target);

/**
 * @brief Appends the source string to the target string.
 *
 * @param source The string to append to the target string.
 * @param target The string to append to the source string.
 * @note The caller is responsible for freeing the returned string.
 * @return The concatenated string, or NULL on failure.
 */
// Semantically, this may differ, but may prove to be identical to concatenation in implementation.
char* flex_string_utf8_string_append(const char* source, const char* target);

// ---------------------- FlexString Operations ----------------------

/**
 * @brief Compare two UTF-8 strings for equality.
 *
 * @param left The left operand string.
 * @param right The right operand string.
 * @return 0 if a == b, -1 if a < b, 1 if a > b, -2 if !a or !b.
 */
int32_t flex_string_compare(const FlexString* left, const FlexString* right);

/**
 * @brief Substitutes all occurrences of a target UTF-8 substring with a replacement substring.
 *
 * @param input The input string.
 * @param target The UTF-8 substring to replace.
 * @param replacement The substring to replace the target with.
 * @note The target and replacement substrings may be any valid sequence of UTF-8 bytes.
 * @return A new string with substitutions applied.
 */
FlexString* flex_string_replace(
    const FlexString* input, const FlexString* target, const FlexString* replacement
);

/**
 * @brief Joins two strings into a new string.
 *
 * @param a The first string.
 * @param b The second string.
 * @return A new string that concatenates `a` and `b`.
 */
char* flex_string_join(const char* a, const char* b);

/**
 * @brief Splits a string into parts based on a delimiter.
 *
 * @param input The input string to split.
 * @param delimiter The delimiter used to split the string.
 * @return A FlexStringSplit object containing the parts of the split string.
 */
FlexStringSplit* flex_string_split(const char* input, const char* delimiter);

/**
 * @brief Tokenizes a string using a regex pattern.
 *
 * @param input The input string.
 * @param pattern The PCRE2 regex pattern.
 * @return A FlexStringSplit object containing the tokens.
 */
FlexStringSplit* flex_string_regex_tokenize(const char* input, const char* pattern);

/**
 * @brief Checks if a string starts with a given prefix.
 *
 * @param input The input string.
 * @param prefix The prefix to check.
 * @return True if the string starts with the prefix, otherwise false.
 */
bool flex_string_starts_with(const char* input, const char* prefix);

/**
 * @brief Checks if a string ends with a given suffix.
 *
 * @param input The input string.
 * @param suffix The suffix to check.
 * @return True if the string ends with the suffix, otherwise false.
 */
bool flex_string_ends_with(const char* input, const char* suffix);

/**
 * @brief Prepends a character to the input string.
 *
 * @param input The input string.
 * @param prepend The UTF-8 string to prepend to the string.
 *
 * @return A new string with the prepended character.
 */
char* flex_string_prepend(const char* input, char* prepend);

/**
 * @brief Appends a character to the input string.
 *
 * @param input The input string.
 * @param append The UTF-8 string to append to the string.
 *
 * @return A new string with the appended character.
 */
char* flex_string_append(const char* input, char* append);

#endif // ALT_FLEX_STRING_H
