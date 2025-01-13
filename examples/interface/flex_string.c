/**
 * @file examples/interface/flex_string.c
 * @brief Example demonstrating flexible string manipulation and tokenization.
 */

#include "interface/logger.h"
#include "interface/flex_string.h"

#include <stdio.h>
#include <stdlib.h>

// Unicode marker representing a space (UTF-8)
#define MARKER "\u2581" // '▁'

// Helper function to print tokens
void print_tokens(FlexStringSplit* tokenizer) {
    if (!tokenizer || tokenizer->length == 0) {
        printf("No tokens found.\n");
        return;
    }

    printf("Found %d tokens:\n", tokenizer->length);
    for (uint32_t i = 0; i < tokenizer->length; i++) {
        if (!tokenizer->parts[i]) {
            LOG_ERROR("%s: Token %d is NULL.\n", __func__, i);
            continue;
        }

        // Substitute spaces with the 'marker'
        char* token_with_marker = flex_string_replace(tokenizer->parts[i], MARKER, " ");
        if (!token_with_marker) {
            LOG_ERROR("%s: Failed to substitute marker in token %d.\n", __func__, i);
            continue;
        }

        printf("Token %d: %s\n", i + 1, token_with_marker);
        free(token_with_marker); // Free the substituted token after use
    }
}

// Main function demonstrating tokenization and substitution
int main(void) {
    const char* text = "The quick brown fox jumps over the lazy dog.";

    // GPT-2 Pre-tokenizer regex pattern
    const char* token_pattern
        = "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+(?!\\S)|\\s+)";

    // Tokenize input text using the provided pattern
    FlexStringSplit* tokenizer = flex_string_regex_tokenize(text, token_pattern);
    if (!tokenizer) {
        LOG_ERROR("%s: Tokenization failed.\n", __func__);
        return 1;
    }

    // Output the tokens using the helper function
    print_tokens(tokenizer);

    // Free the memory allocated for the tokens and the FlexString object
    flex_string_free_split(tokenizer);

    return 0;
}
