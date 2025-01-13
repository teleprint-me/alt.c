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

// ---------------------- Structures ----------------------

/**
 * @brief Represents a mutable string with dynamic memory management.
 */
typedef struct {
    char* data; ///< Pointer to the string data.
    uint32_t length; ///< Length of the string (in characters, not bytes).
} FlexString;

/**
 * @brief Represents a flexible string with multiple parts.
 */
typedef struct {
    char** parts; ///< Array of split strings.
    uint32_t length; ///< Number of parts (strings) in the array.
} FlexStringSplit;

// ---------------------- Lifecycle Functions ----------------------

/**
 * @brief Creates a FlexString with the given data.
 *
 * @param data The initial string data.
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
 * @return Pointer to a newly allocated FlexStringSplit object.
 */
FlexStringSplit* flex_string_create_split(void);

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
 * @brief Glues the bytes of a UTF-8 character into a null-terminated string.
 *
 * This function takes a pointer to a UTF-8 character (represented as a sequence of bytes) and
 * returns a new string containing the character's bytes. The resulting string is null-terminated.
 *
 * @param string The input string containing the bytes of the UTF-8 character.
 * @param char_length The length of the input character.
 * @return A newly allocated null-terminated string containing the bytes of the UTF-8 character,
 *         or NULL if memory allocation fails.
 * @note The caller is responsible for freeing the returned string.
 */
char* flex_string_utf8_char_glue(const uint8_t* string, int8_t char_length);

// ---------------------- String Operations ----------------------

/**
 * @brief Validates an entire UTF-8 string.
 *
 * This function iterates through the input string and validates each UTF-8 character
 * using the UTF-8 character encoding rules.
 *
 * @param input The input string to be validated.
 * @return true if the string is valid UTF-8, false otherwise.
 */
bool flex_string_validate(const char* input);

/**
 * @brief Calculates the length of a UTF-8 string in characters.
 *
 * @param input The input string.
 * @return The number of UTF-8 characters in the string.
 */
int32_t flex_string_length(const char* input);

/**
 * @brief Substitutes all occurrences of a target UTF-8 character with a replacement string.
 *
 * @param input The input string.
 * @param target The UTF-8 character to replace.
 * @param replacement The string to replace the target with.
 * @return A new string with substitutions applied.
 */
char* flex_string_replace(const char* input, const char* replacement, const char* target);

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
