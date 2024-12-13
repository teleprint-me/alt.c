/**
 * @file examples/scratchpad/u8proc.c
 *
 * @brief Simple usage example for utf8proc, demonstrating UTF-8 validation,
 * normalization, and grapheme cluster analysis.
 *
 * @note The utf8proc library defines custom typedefs (e.g., `utf8proc_uint8_t` 
 * and `utf8proc_int32_t`) as aliases for standard C types like `uint8_t` and 
 * `int32_t`. These typedefs were historically introduced for compatibility 
 * with older compilers and environments lacking C99 support, such as MSVC 
 * versions before 2013. On modern platforms, these typedefs are functionally 
 * equivalent to their standard counterparts but align with the library's 
 * naming conventions.
 *
 * @note The `utf8proc_ssize_t` typedef serves as a signed size type and maps 
 * to `ptrdiff_t` on most platforms. On Windows, it maps to `__int64` for 
 * 64-bit systems or `int` for 32-bit systems. This ensures consistent handling 
 * of signed sizes across platforms, including legacy environments.
 *
 * @ref https://github.com/JuliaStrings/utf8proc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <utf8proc.h>

typedef utf8proc_ssize_t ssize_t; // alias for ptrdiff_t

void grapheme_clusters(const char* input) {
    const uint8_t* str = (const uint8_t*) input;
    int32_t codepoint1, codepoint2;
    int32_t state = 0;

    // Read the first codepoint
    ssize_t bytes_read = utf8proc_iterate(str, -1, &codepoint1);
    if (bytes_read < 0) {
        printf("Invalid UTF-8 input.\n");
        return;
    }
    str += bytes_read;

    while (*str) {
        // Read the next codepoint
        bytes_read = utf8proc_iterate(str, -1, &codepoint2);
        if (bytes_read < 0) {
            printf("Invalid UTF-8 sequence during grapheme parsing.\n");
            return;
        }

        // Determine grapheme break
        int grapheme_break = utf8proc_grapheme_break_stateful(codepoint1, codepoint2, &state);
        printf(
            "Codepoint: U+%04X (%c), Grapheme Break: %d\n",
            codepoint1,
            (char) codepoint1,
            grapheme_break
        );

        // Move to the next codepoint
        codepoint1 = codepoint2;
        str += bytes_read;
    }

    // Print the last codepoint (no next codepoint for comparison)
    printf("Codepoint: U+%04X (%c), Grapheme Break: 1\n", codepoint1, (char) codepoint1);
}

void test_utf8proc(const char* input) {
    const uint8_t* str = (const uint8_t*) input;
    int32_t codepoint = 0;

    // Step 1: Validate UTF-8
    printf("Input: %s\n", input);
    if (utf8proc_iterate(str, -1, &codepoint) < 0) {
        printf("Invalid UTF-8 sequence in input.\n");
        return;
    } else {
        printf("Valid UTF-8 sequence.\n");
    }

    // Step 2: Normalize to NFC
    uint8_t* normalized = utf8proc_NFC(str);
    if (normalized) {
        printf("Normalized (NFC): %s\n", normalized);
        free(normalized);
    } else {
        printf("Normalization failed.\n");
    }

    // Step 3: Analyze Grapheme Clusters
    printf("Grapheme Clusters:\n");
    grapheme_clusters(input);
}

int main() {
    const char* example = "Hello, 世界! 😀 🚀";
    test_utf8proc(example);
    return 0;
}
