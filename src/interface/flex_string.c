/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/interface/flex_string.c
 * @brief Flexible String API for common ASCII and UTF-8 string manipulation.
 *
 * Provides utilities for working with UTF-8 strings, including splitting,
 * joining, substitution, and regex-based operations.
 */

#include "interface/flex_string.h"
#include "interface/logger.h"

// ---------------------- Lifecycle Functions ----------------------

FlexString* flex_string_create(char* data) {
    if (!data || *data == '\0') {
        LOG_ERROR("%s: Input data is NULL\n", __func__);
        return NULL;
    }

    FlexString* string = malloc(sizeof(FlexString));
    if (!string) {
        LOG_ERROR("%s: Failed to allocate memory to string.\n", __func__);
        return NULL;
    }

    // Validate input data
    if (flex_string_validate(data) == false) {
        LOG_ERROR("%s: Illegal sequence detected: Cowardly refusing to continue.\n", __func__);
        free(string);
        return NULL;
    }

    // Calculate the length of the input string
    string->length = flex_string_length(data);
    // Allocate memory for the string data (including null terminator)
    string->data = malloc((string->length + 1) * sizeof(char));
    if (!string->data) {
        LOG_ERROR("%s: Failed to allocate memory to string data.\n", __func__);
        free(string);
        return NULL;
    }

    // Copy the input string to the newly allocated memory
    for (uint32_t i = 0; i < string->length; i++) {
        string->data[i] = data[i];
    }
    string->data[string->length] = '\0'; // Null-terminate the string

    return string;
}

void flex_string_free(FlexString* string) {
    if (string) {
        if (string->data) {
            free(string->data);
        }
        free(string);
    }
}

FlexStringSplit* flex_string_create_split(void) {
    FlexStringSplit* split = (FlexStringSplit*) malloc(sizeof(FlexStringSplit));
    if (!split) {
        LOG_ERROR("%s: Failed to allocate memory to FlexStringSplit\n", __func__);
        return NULL;
    }
    split->parts = NULL; // Use realloc() at runtime
    split->length = 0;
    return split;
}

void flex_string_free_split(FlexStringSplit* split) {
    if (split) {
        if (split->parts) {
            for (uint32_t i = 0; i < split->length; i++) {
                if (split->parts[i]) {
                    free(split->parts[i]);
                }
            }
            free(split->parts);
        }
        free(split);
    }
}

// ---------------------- Character Operations ----------------------

int8_t flex_string_utf8_char_length(uint8_t byte) {
    if ((byte & 0x80) == 0) {
        return 1;
    } else if ((byte & 0xE0) == 0xC0) {
        return 2;
    } else if ((byte & 0xF0) == 0xE0) {
        return 3;
    } else if ((byte & 0xF8) == 0xF0) {
        return 4;
    } else {
        return -1;
    }
}

bool flex_string_utf8_char_validate(const uint8_t* string, int8_t char_length) {
    if (!string || '\0' == *string || 0 >= char_length) {
        return false;
    }

    if (char_length == 1) {
        // ASCII (1-byte) characters are always valid
        return true;
    }

    // Validate continuation bytes for multi-byte characters
    for (int8_t i = 1; i < char_length; i++) {
        if ((string[i] & 0xC0) != 0x80) {
            return false; // Invalid continuation byte
        }
    }

    // Additional checks for overlong encodings and invalid ranges
    if (char_length == 2) {
        if (string[0] < 0xC2) {
            return false; // Overlong encoding
        }
    } else if (char_length == 3) {
        if (string[0] == 0xE0 && string[1] < 0xA0) {
            return false; // Overlong encoding
        }
        if (string[0] == 0xED && string[1] >= 0xA0) {
            return false; // Surrogate halves
        }
    } else if (char_length == 4) {
        if (string[0] == 0xF0 && string[1] < 0x90) {
            return false; // Overlong encoding
        }
        if (string[0] == 0xF4 && string[1] > 0x8F) {
            return false; // Above U+10FFFF
        }
    }

    // If all checks passed, the character is valid
    return true;
}

// Glue bytes of a UTF-8 character into a null-terminated string
char* flex_string_utf8_char_glue(const uint8_t* string, int8_t char_length) {
    if (!string || *string == '\0' || 1 >= char_length) {
        LOG_ERROR("%s: Invalid byte sequence or length.\n", __func__);
        return NULL;
    }

    char* result = malloc(char_length + 1); // +1 for null terminator
    if (!result) {
        LOG_ERROR("%s: Failed to allocate memory.\n", __func__);
        return NULL;
    }

    memcpy(result, string, char_length);
    result[char_length] = '\0'; // Null-terminate the string
    return result;
}

// ---------------------- String Operations ----------------------

bool flex_string_validate(const char* input) {
    if (!input) {
        return false; // Null input is invalid
    }

    const uint8_t* stream = (const uint8_t*) input;
    while (*stream) {
        // Determine the length of the current UTF-8 character
        int8_t char_length = flex_string_utf8_char_length(*stream);
        if (char_length == -1) {
            LOG_ERROR("Invalid UTF-8 leading byte: 0x%02X\n", *stream);
            return false;
        }

        // Validate the UTF-8 character
        if (!flex_string_utf8_char_validate(stream, char_length)) {
            LOG_ERROR("Invalid UTF-8 sequence starting at byte: 0x%02X\n", *stream);
            return false;
        }

        // Move the pointer forward by the character length
        stream += char_length;
    }

    return true;
}

int32_t flex_string_length(const char* input) {
    if (!input) {
        return -1; // Null input is invalid
    }

    if (*input == '\0') {
        return 0; // This is a valid length (i think?)
    }

    int32_t char_count = 0;
    const unsigned char* stream = (const unsigned char*) input;

    while (*stream) {
        // Determine the length of the UTF-8 character
        int char_length = flex_string_utf8_char_length(*stream);
        if (char_length == -1) {
            LOG_ERROR("Invalid UTF-8 leading byte: 0x%02X\n", *stream);
            return -1;
        }

        // Validate the UTF-8 character directly
        if (!flex_string_utf8_char_validate(stream, char_length)) {
            LOG_ERROR("Invalid UTF-8 sequence starting at byte: 0x%02X\n", *stream);
            return -1;
        }

        // Move the pointer forward by the character length
        stream += char_length;

        // Increment the character count
        char_count++;
    }

    return char_count;
}

/**
 * @brief Substitutes all occurrences of a target UTF-8 character with a replacement string.
 *
 * @param input The input string.
 * @param target The UTF-8 character to replace.
 * @param replacement The string to replace the target with.
 * @return A new string with substitutions applied.
 */
char* flex_string_replace(const char* input, const char* replacement, const char* target) {
    // Ensure input strings are not NULL
    if (!input || !target) {
        LOG_ERROR("%s: Invalid input string\n", __func__);
        return NULL;
    }
    // Ensure target is a valid UTF-8 character
    if (true != flex_string_validate(target)) {
        LOG_ERROR("%s: Invalid target character\n", __func__);
        return NULL;
    }
    // Ensure replacement is a valid UTF-8 character
    if (true != flex_string_validate(replacement)) {
        LOG_ERROR("%s: Invalid replacement character\n", __func__);
        return NULL;
    }
    // Ensure target and replacement are not the same
    return; // TODO: Handle error case
}

char* flex_string_join(const char* a, const char* b) {
    // Ensure input strings are not NULL
    if (!a || !b) {
        LOG_ERROR("%s: Invalid input string\n", __func__);
        return NULL;
    }

    // Allocate memory for the new string (+1 for the null terminator)
    size_t len = strlen(a) + strlen(b) + 1;
    char* result = (char*) malloc(len);

    if (!result) {
        LOG_ERROR("%s: Memory allocation failed\n", __func__);
        return NULL;
    }

    // Concatenate the strings
    strcpy(result, a);
    strcat(result, b);

    return result;
}

FlexStringSplit* flex_string_split(const char* input, const char* delimiter) {
    if (!input || !delimiter){
        LOG_ERROR("%s: Invalid input or delimiter\n", __func__);
        return NULL;
    }
    if (!flex_string_validate(input)){
        LOG_ERROR("%s: Invalid input string\n", __func__);
        return NULL;
    }

    FlexStringSplit* split = flex_string_create_split();
    if (!split){
        LOG_ERROR("%s: Failed to allocate memory\n", __func__);
        return NULL;
    }

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

// FlexString* flex_string_create_tokens(const char* input, const char* pattern) {
//     FLEX_STRING_GUARD(input, pattern);

//     pcre2_code* re;
//     PCRE2_SIZE erroffset;
//     int errorcode;
//     PCRE2_UCHAR8 buffer[256];

//     re = pcre2_compile(
//         (PCRE2_SPTR) pattern,
//         PCRE2_ZERO_TERMINATED,
//         PCRE2_UTF | PCRE2_UCP,
//         &errorcode,
//         &erroffset,
//         NULL
//     );

//     if (!re) {
//         pcre2_get_error_message(errorcode, buffer, sizeof(buffer));
//         LOG_ERROR("%s: PCRE2 compilation failed at offset %zu: %s\n", __func__, erroffset,
//         buffer); return NULL;
//     }

//     pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(re, NULL);
//     if (!match_data) {
//         LOG_ERROR("%s: Failed to create match data\n", __func__);
//         pcre2_code_free(re);
//         return NULL;
//     }

//     const char* cursor = input;
//     size_t subject_length = strlen(input);
//     FlexString* token = flex_string_create();
//     if (!token) {
//         pcre2_match_data_free(match_data);
//         pcre2_code_free(re);
//         return NULL;
//     }

//     while (subject_length > 0) {
//         int rc = pcre2_match(re, (PCRE2_SPTR) cursor, subject_length, 0, 0, match_data, NULL);
//         if (rc <= 0) {
//             break;
//         }

//         PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);
//         size_t match_length = ovector[1] - ovector[0];

//         char* part = strndup(cursor + ovector[0], match_length);
//         if (!part) {
//             break;
//         }

//         char** temp = realloc(token->parts, sizeof(char*) * (token->length + 1));
//         if (!temp) {
//             free(part);
//             break;
//         }
//         token->parts = temp;
//         token->parts[token->length++] = part;

//         cursor += ovector[1];
//         subject_length -= ovector[1];
//     }

//     pcre2_match_data_free(match_data);
//     pcre2_code_free(re);

//     return token;
// }

// char* flex_string_substitute_char(const char* input, const char* replacement, char target) {
//     FLEX_STRING_GUARD(input, replacement);

//     size_t replacement_length = strlen(replacement);

//     // Calculate new string length and build result in one pass
//     size_t result_length = 0;
//     const char* src = input;
//     while (*src) {
//         result_length += (*src == target) ? replacement_length : 1;
//         src++;
//     }

//     char* result = (char*) malloc(result_length + 1);
//     if (!result) {
//         LOG_ERROR("%s: Failed to allocate memory\n", __func__);
//         return NULL;
//     }

//     // Build the resulting string
//     const char* source_cursor = input;
//     char* result_cursor = result;
//     while (*source_cursor) {
//         if (*source_cursor == target) {
//             strcpy(result_cursor, replacement);
//             result_cursor += replacement_length;
//         } else {
//             *result_cursor++ = *source_cursor;
//         }
//         source_cursor++;
//     }
//     *result_cursor = '\0';

//     return result;
// }

// // Function to prepend a character
// char* flex_string_prepend_char(const char* input, char prepend) {
//     size_t input_len = strlen(input);
//     char* result = (char*) malloc(input_len + 2); // +2 for the prepend char and null terminator
//     if (!result) {
//         perror("Failed to allocate memory");
//         exit(EXIT_FAILURE);
//     }

//     result[0] = prepend;
//     strcpy(result + 1, input);
//     return result;
// }

// // Function to append a character
// char* flex_string_append_char(const char* input, char append) {
//     size_t input_len = strlen(input);
//     char* result = (char*) malloc(input_len + 2); // +2 for the append char and null terminator
//     if (!result) {
//         perror("Failed to allocate memory");
//         exit(EXIT_FAILURE);
//     }

//     strcpy(result, input);
//     result[input_len] = append;
//     result[input_len + 1] = '\0';
//     return result;
// }
