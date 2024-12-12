/**
 * @file examples/scratchpad/utf8.c
 * 
 * @brief For simple tasks, UTF-8 just works as long as the characters are treated as strings.
 */

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Example tokenization function
void tokenize_utf8(const char* text) {
    const char* delimiters = " .,!?;:\"()";
    char* text_copy = strdup(text); // Make a mutable copy
    char* token = strtok(text_copy, delimiters);

    while (token) {
        printf("Token: %s\n", token);
        token = strtok(NULL, delimiters);
    }

    free(text_copy);
}

int main() {
    setlocale(LC_ALL, ""); // Enable UTF-8 handling

    const char* text = "Hello, world! 你好，世界! 😀 🚀";
    printf("Original text: %s\n", text);

    printf("Tokens:\n");
    tokenize_utf8(text);

    return 0;
}
