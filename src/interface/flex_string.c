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
    if (!data) {
        LOG_ERROR("%s: Input data is NULL.\n", __func__);
        return NULL;
    }

    // Validate the input string
    if (!flex_string_utf8_string_validate(data)) {
        LOG_ERROR("%s: Input data is not a valid UTF-8 string.\n", __func__);
        return NULL;
    }

    FlexString* string = malloc(sizeof(FlexString));
    if (!string) {
        LOG_ERROR("%s: Failed to allocate memory for FlexString.\n", __func__);
        return NULL;
    }

    // Logs errors if not successful
    uint32_t length = flex_string_utf8_string_char_length(data); // Character length
    uint32_t capacity = flex_string_utf8_string_byte_length(data); // Byte length + null terminator
    string->data = flex_string_utf8_string_copy(data); // Alloc and copy
    if (!string->data) {
        flex_string_free(string);
        return NULL;
    }

    string->length = length;
    string->capacity = capacity;
    string->valid_utf8 = 1; // Already validated
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

FlexStringSplit* flex_string_create_split(uint32_t initial_capacity) {
    if (initial_capacity == 0) {
        initial_capacity = 4; // Default capacity
    }

    FlexStringSplit* split = malloc(sizeof(FlexStringSplit));
    if (!split) {
        LOG_ERROR("%s: Failed to allocate memory for FlexStringSplit.\n", __func__);
        return NULL;
    }

    split->parts = malloc(initial_capacity * sizeof(char*));
    if (!split->parts) {
        LOG_ERROR("%s: Failed to allocate memory for FlexStringSplit parts.\n", __func__);
        free(split);
        return NULL;
    }

    split->length = 0;
    split->capacity = initial_capacity;
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

// ---------------------- UTF-8 Character Operations ----------------------

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

void* flex_string_utf8_char_iterator(
    const char* input, FlexStringUTF8Iterator callback, void* context
) {
    if (!input || !callback) {
        return NULL; // Invalid input or callback
    }

    const uint8_t* stream = (const uint8_t*) input;
    while (*stream) {
        // Determine the length of the current UTF-8 character
        int8_t char_length = flex_string_utf8_char_length(*stream);

        if (char_length == -1 || !flex_string_utf8_char_validate(stream, char_length)) {
            // Notify the callback of an invalid sequence and allow it to decide
            void* result = callback(stream, -1, context);
            if (result) {
                return result; // Early return based on callback result
            }
            stream++; // Move past the invalid byte to prevent infinite loops
            continue;
        }

        // Invoke the callback with the current character
        void* result = callback(stream, char_length, context);
        if (result) {
            return result; // Early return based on callback result
        }

        stream += char_length; // Advance to the next character
    }

    return NULL; // Completed iteration without finding a result
}

// ---------------------- UTF-8 String Validation ----------------------

typedef struct UTF8ValidationContext {
    bool is_valid; // Overall validity of the input
    const uint8_t* error_at; // Pointer to the first invalid byte, if any
} UTF8ValidationContext;

void* utf8_char_validator(const uint8_t* char_start, int8_t char_length, void* context) {
    UTF8ValidationContext* validator = (UTF8ValidationContext*) context;

    if (char_length == -1) {
        // Invalid UTF-8 sequence detected
        validator->is_valid = false;
        validator->error_at = char_start; // Capture the error location
        return (void*) validator; // Stop iteration immediately
    }

    validator->is_valid = true; // Mark as valid for this character
    return NULL; // Continue iteration
}

bool flex_string_utf8_string_validate(const char* input) {
    if (!input) {
        return false;
    }

    UTF8ValidationContext validator = {
        .is_valid = true,
        .error_at = NULL,
    };

    flex_string_utf8_char_iterator(input, utf8_char_validator, &validator);

    if (!validator.is_valid && validator.error_at) {
        LOG_ERROR(
            "Invalid UTF-8 sequence detected at byte offset: %ld\n",
            validator.error_at - (const uint8_t*) input
        );
    }

    return validator.is_valid;
}

// ---------------------- UTF-8 String Length ----------------------

typedef struct UTF8Length {
    int32_t value;
} UTF8Length;

// Callback to validate each character
void* utf8_char_counter(const uint8_t* char_start, int8_t char_length, void* context) {
    (void) char_start;
    (void) char_length;
    UTF8Length* length = (UTF8Length*) context;
    length->value++;
    return NULL; // Continue iteration as long as the input is valid
}

int32_t flex_string_utf8_string_char_length(const char* input) {
    if (!input) {
        return -1;
    }
    if ('\0' == *input) {
        return 0; // Empty string
    }
    UTF8Length length = {.value = 0};
    if (flex_string_utf8_char_iterator(input, utf8_char_counter, &length) == NULL) {
        return length.value;
    }
    return -1;
}

// ---------------------- UTF-8 String Byte Length ----------------------

int32_t flex_string_utf8_string_byte_length(const char* input) {
    if (!input) {
        return -1; // Null input
    }
    if (!flex_string_utf8_string_validate(input)) {
        return -1; // Invalid UTF-8 string
    }
    if ('\0' == *input) {
        return 0; // Empty string
    }
    // Track the byte length of the string
    uint32_t byte_length = 0;
    // Iterate through each byte in the string
    const char* stream = input;
    while (*stream) {
        byte_length++; // increment the byte length counter
        stream++; // move to the next byte in the string
    }
    return byte_length;
}

// ---------------------- UFT-8 String Compare ----------------------

int32_t flex_string_utf8_string_compare(const char* first, const char* second) {
    // 0 if a == b, -1 if a < b, 1 if a > b, -2 if !a or !b.
    if (!first || !second) {
        LOG_ERROR("%s: One or both input strings are NULL.\n", __func__);
        return FLEX_STRING_COMPARE_INVALID; // NULL strings are invalid inputs.
    }
    if (!flex_string_utf8_string_validate(first)) {
        LOG_ERROR("%s: First input string is not a valid UTF-8 string.\n", __func__);
        return FLEX_STRING_COMPARE_INVALID; // Indicate invalid UTF-8 string
    }
    if (!flex_string_utf8_string_validate(second)) {
        LOG_ERROR("%s: Second input string is not a valid UTF-8 string.\n", __func__);
        return FLEX_STRING_COMPARE_INVALID; // Indicate invalid UTF-8 string
    }

    // Make the input strings immutable.
    const char* first_stream = first;
    const char* second_stream = second;

    // Compare stream objects.
    while (*first_stream && *second_stream) {
        if (*first_stream < *second_stream) {
            return FLEX_STRING_COMPARE_LESS;
        }
        if (*first_stream > *second_stream) {
            return FLEX_STRING_COMPARE_GREATER;
        }

        // Both bytes are equal, move to the next
        first_stream++;
        second_stream++;
    }

    // Check for string length differences
    if (*first_stream) {
        return FLEX_STRING_COMPARE_GREATER;
    }
    if (*second_stream) {
        return FLEX_STRING_COMPARE_LESS;
    }

    return FLEX_STRING_COMPARE_EQUAL;
}

// ---------------------- UFT-8 String Copy ----------------------

char* flex_string_utf8_string_copy(const char* input) {
    if (!input) {
        LOG_ERROR("%s: Invalid input string.\n", __func__);
        return NULL;
    }
    if (false == flex_string_utf8_string_validate(input)) {
        LOG_ERROR("%s: Invalid input string.\n", __func__);
        return NULL;
    }

    // Calculate the byte length of the input string
    size_t capacity = flex_string_utf8_string_byte_length(input);
    if (capacity == 0) {
        LOG_ERROR("%s: Empty input string.\n", __func__);
        return NULL;
    }

    // Allocate memory for the output string with an extra byte for null terminator
    char* output = malloc((capacity + 1) * sizeof(char));
    if (!output) {
        LOG_ERROR("%s: Failed to allocate memory for output string.\n", __func__);
        return NULL;
    }

    // Manually handle bytes to ensure every byte is copied according to the given capacity.
    for (uint32_t i = 0; i < capacity; i++) {
        output[i] = input[i];
    }
    output[capacity] = '\0'; // Null-terminate the string

    return output;
}

// ---------------------- UFT-8 String Concatenation ----------------------

char* flex_string_utf8_string_concat(const char* left, const char* right) {
    // Check for null pointers and empty strings
    if (!left || !right || '\0' == *left || '\0' == *right) {
        LOG_ERROR("%s: Invalid input parameters\n", __func__);
        return NULL;
    }
    // Validate the left and right operands.
    if (!flex_string_utf8_string_validate(left)) {
        LOG_ERROR("%s: Invalid left operand\n", __func__);
        return NULL;
    }
    if (!flex_string_utf8_string_validate(right)) {
        LOG_ERROR("%s: Invalid right operand\n", __func__);
        return NULL;
    }
    // Concatenate the right operand to the left operand.
    size_t left_len = flex_string_utf8_string_byte_length(left);
    size_t right_len = flex_string_utf8_string_byte_length(right);
    size_t dest_len = left_len + right_len + 1; // +1 for the null terminator
    char* output = (char*) malloc(dest_len);
    if (output == NULL) {
        return NULL;
    }

    // Copy string bytes into output
    memcpy(output, left, left_len);
    memcpy(output + left_len, right, right_len);
    output[left_len + right_len] = '\0'; // Null-terminate the string

    return output;
}

// ---------------------- UFT-8 String Operations ----------------------

// FlexString* flex_string_replace(const FlexString* input, const FlexString* replacement, const
// FlexString* target) {
//     // Validate input
//     if (!input || !input->data || !replacement || !replacement->data || !target || !target->data)
//     {
//         LOG_ERROR("%s: Invalid input, replacement, or target string.\n", __func__);
//         return NULL;
//     }

//     // If the target and replacement are the same, return a copy of the input
//     if (flex_string_compare(target, replacement) == 0) {
//         return flex_string_copy(input, strlen(input));
//     }

//     // Determine lengths of input, target, and replacement
//     size_t input_len = flex_string_length(input);
//     size_t target_len = flex_string_length(target);
//     size_t replacement_len = flex_string_length(replacement);

//     // Allocate buffer for the result (estimate: input_len + some extra space)
//     char* result = malloc(input_len + 1); // Initial allocation
//     if (!result) {
//         LOG_ERROR("%s: Failed to allocate memory for the result string.\n", __func__);
//         return NULL;
//     }

//     size_t result_capacity = input_len + 1;
//     size_t result_len = 0;

//     const char* current = input;
//     while (*current) {
//         // Check if the current substring matches the target
//         if (flex_string_compare(current, target) == 0) {
//             // Ensure enough space in the result buffer
//             size_t new_capacity = result_len + replacement_len + 1;
//             if (new_capacity > result_capacity) {
//                 result_capacity = new_capacity * 2; // Double the capacity
//                 char* temp = realloc(result, result_capacity);
//                 if (!temp) {
//                     LOG_ERROR("%s: Memory allocation failed during resizing.\n", __func__);
//                     free(result);
//                     return NULL;
//                 }
//                 result = temp;
//             }

//             // Append the replacement
//             if (replacement) {
//                 result = flex_string_copy(replacement, replacement_len);
//                 result_len += replacement_len;
//             }

//             // Skip past the target in the input
//             current += target_len;
//         } else {
//             // Copy the current character into the result
//             result[result_len++] = *current++;
//         }
//     }

//     // Null-terminate the result
//     result[result_len] = '\0';

//     return result;
// }

FlexStringSplit* flex_string_split(const char* input, const char* delimiter) {
    if (!input || !delimiter) {
        LOG_ERROR("%s: Invalid input or delimiter\n", __func__);
        return NULL;
    }
    if (!flex_string_utf8_string_validate(input)) {
        LOG_ERROR("%s: Invalid input string\n", __func__);
        return NULL;
    }

    // Initial capacity is 0
    FlexStringSplit* split = flex_string_create_split(0);
    if (!split) {
        LOG_ERROR("%s: Failed to allocate memory\n", __func__);
        return NULL;
    }

    char* temp = flex_string_utf8_string_copy(input);
    char* token = strtok(temp, delimiter);
    while (token) {
        split->parts = realloc(split->parts, (split->length + 1) * sizeof(char*));
        if (!split->parts) {
            free(temp);
            free(split);
            return NULL;
        }
        split->parts[split->length] = flex_string_utf8_string_copy(token);
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
