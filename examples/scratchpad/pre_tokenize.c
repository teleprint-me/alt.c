// must be defined before including pcre2.h
#define PCRE2_CODE_UNIT_WIDTH 8

#include <pcre2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define REGEX_PATTERN \
    "('s|'t|'re|'ve|'m|'ll|'d| ?\\p{L}+| ?\\p{N}+| ?[^\\s\\p{L}\\p{N}]+|\\s+(?!\\S)|\\s+)"

void mistral_pre_tokenize(const char* input) {
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
        return;
    }

    pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(re, NULL);

    const char* cursor = input;
    size_t subject_length = strlen(input);

    while (*cursor) {
        int rc = pcre2_match(re, (PCRE2_SPTR) cursor, subject_length, 0, 0, match_data, NULL);

        if (rc > 0) {
            PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);

            // Process the primary match (ovector[0] and ovector[1])
            PCRE2_SPTR start = (PCRE2_SPTR) cursor + ovector[0];
            PCRE2_SIZE length = ovector[1] - ovector[0];

            char* token = strndup((const char*) start, length);
            printf("Token: '%s'\n", token);
            free(token);

            // Advance cursor past the current match
            cursor += ovector[1];
            subject_length -= ovector[1];
        } else {
            break; // No more matches
        }
    }

    pcre2_match_data_free(match_data);
    pcre2_code_free(re);
}

int main() {
    const char* test_string = "Once upon a time, a wizard lived in the forest...";
    mistral_pre_tokenize(test_string);
    return 0;
}
