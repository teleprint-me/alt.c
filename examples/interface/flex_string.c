/**
 * @file examples/interface/flex_string.c
 */

#include "interface/flex_string.h"

#include <stdio.h>
#include <stdlib.h>

// Google's meta marker representing a space (UTF-8)
#define MARKER "\u2581" // UTF-8 marker '▁'

int main() {
    const char* text = "The quick brown fox jumps over the lazy dog.";

    // GPT-2 Pre-tokenizer regex pattern
    const char* token_pattern
        = "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+(?!\\S)|\\s+)";

    FlexString* tokenizer = flex_string_create_tokens(text, token_pattern);

    if (!tokenizer) {
        LOG_ERROR("%s: Tokenization failed.\n", __func__);
        return 1;
    }

    // Output the tokens
    printf("Found %zu tokens:\n", tokenizer->length);
    for (size_t i = 0; i < tokenizer->length; i++) {
        if (!tokenizer->parts[i]) {
            LOG_ERROR("%s: Token %zu is NULL.\n", __func__, i);
            continue;
        }

        // Substitute spaces with the 'marker'
        char* token_with_marker = flex_string_substitute_char(tokenizer->parts[i], MARKER, ' ');
        if (token_with_marker == NULL) {
            LOG_ERROR("%s: Failed to substitute marker in token %zu.\n", __func__, i);
            free(tokenizer->parts[i]);
            continue;
        }

        printf("Token %zu: %s\n", i + 1, token_with_marker);
        free(token_with_marker); // Free the substituted token
    }

    // Free the tokens array
    flex_string_free(tokenizer);

    return 0;
}
