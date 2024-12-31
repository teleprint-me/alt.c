/**
 * @file examples/scratchpad/pre_tokenize.c
 */

// must be defined before including pcre2.h
#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGEX_PATTERN \
    "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+(?!\\S)|\\s+)"

/**
 * @brief Tokenizes the input text into an array of strings.
 *
 * @param input The input string to tokenize.
 * @return An array of token strings, NULL-terminated. The caller is responsible for freeing the
 * memory.
 */
char** mistral_pre_tokenize(const char* input) {
    pcre2_code* re;
    PCRE2_SIZE erroffset;
    int errorcode;
    PCRE2_UCHAR8 buffer[256];

    re = pcre2_compile(
        (PCRE2_SPTR) REGEX_PATTERN,
        PCRE2_ZERO_TERMINATED,
        PCRE2_UTF | PCRE2_UCP, // UTF-8 and Unicode properties
        &errorcode,
        &erroffset,
        NULL
    );

    if (!re) {
        pcre2_get_error_message(errorcode, buffer, sizeof(buffer));
        fprintf(stderr, "PCRE2 compilation failed at offset %zu: %s\n", erroffset, buffer);
        return NULL;
    }

    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(re, NULL);

    const char* cursor = input;
    size_t subject_length = strlen(input);

    // Dynamic array to hold tokens
    char** tokens = NULL;
    size_t token_count = 0;

    while (*cursor) {
        int rc = pcre2_match(re, (PCRE2_SPTR) cursor, subject_length, 0, 0, match_data, NULL);

        if (rc > 0) {
            PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);

            // Process the primary match (ovector[0] and ovector[1])
            PCRE2_SPTR start = (PCRE2_SPTR) cursor + ovector[0];
            PCRE2_SIZE length = ovector[1] - ovector[0];

            char* token = strndup((const char*) start, length);
            if (!token) {
                fprintf(stderr, "Memory allocation failed for token.\n");
                break;
            }

            // Add token to the dynamic array
            tokens = realloc(tokens, sizeof(char*) * (token_count + 2));
            if (!tokens) {
                fprintf(stderr, "Memory allocation failed for token array.\n");
                free(token);
                break;
            }
            tokens[token_count++] = token;
            tokens[token_count] = NULL; // NULL-terminate the array

            // Advance cursor past the current match
            cursor += ovector[1];
            subject_length -= ovector[1];
        } else {
            break; // No more matches
        }
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);

    return tokens;
}

int main() {
    const char* test_string = "Once upon a time, a wizard lived in the forest...";
    char** tokens = mistral_pre_tokenize(test_string);

    if (tokens) {
        printf("Tokens:\n");
        for (size_t i = 0; tokens[i]; i++) {
            printf("Token: '%s'\n", tokens[i]);
            free(tokens[i]); // Free each token
        }
        free(tokens); // Free the token array
    }

    return 0;
}
