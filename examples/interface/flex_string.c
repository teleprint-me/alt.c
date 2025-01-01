/**
 * @file examples/interface/flex_string.c
 */

#include "interface/flex_string.h"

#include <stdio.h>
#include <stdlib.h>

#define MARKER "\xe2\x96\x81" // UTF-8 marker '▁'

int main(void) {
    char* source_string = "The quick brown fox jumped over the lazy dog.";
    char* target_string = flex_string_sub_char_with_uft8(source_string, MARKER, ' ');
    printf("Source String: %s\n", source_string);
    printf("Target String: %s\n", target_string);
    free(target_string);
    return EXIT_FAILURE;
}
