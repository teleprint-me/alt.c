/**
 * @file examples/interface/flex_string.c
 */

#include "interface/flex_string.h"

#include <stdio.h>
#include <stdlib.h>

// Googles meta marker representing a space
#define MARKER "\u2581" // UTF-8 marker '▁'

int main() {
    const char* text = "The quick brown fox jumps over the lazy dog.";
    // const char* regex = "\\w+";
    // GPT-2 Pre-tokenizer
    const char* regex
        = "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+(?!\\S)|\\s+)";

    size_t token_count = 0;
    char** tokens = flex_string_tokenize(text, regex, &token_count);

    if (tokens) {
        printf("Found %zu tokens:\n", token_count);
        for (size_t i = 0; i < token_count; i++) {
            char* token = flex_string_substitute(tokens[i], MARKER, ' ');
            printf("Token %zu: %s\n", i + 1, token);
            free(token);
            free(tokens[i]);
        }
        free(tokens);
    }

    return 0;
}
