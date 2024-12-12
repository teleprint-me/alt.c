/**
 * @file examples/scratchpad/char.c
 *
 * @brief Prototype for auto-generating input characters.
 *
 * @ref https://www.cprogramming.com/tutorial/unicode.html
 */

#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Example UTF-8 strings
const char* utf8_chars[] = {
    "A", "B", "C", "\u03B1", "\u03B2", "\u03B3", "😀", "🚀", "\u263A", "你好", "こんにちは"
};

typedef struct Dataset {
    size_t length;
    char** samples; // Array of UTF-8 strings
} Dataset;

Dataset* dataset_create() {
    size_t count = sizeof(utf8_chars) / sizeof(utf8_chars[0]);
    Dataset* dataset = (Dataset*) malloc(sizeof(Dataset));

    if (!dataset) {
        return NULL;
    }

    dataset->length = count;
    dataset->samples = (char**) malloc(sizeof(char*) * count);

    if (!dataset->samples) {
        free(dataset);
        return NULL;
    }

    for (size_t i = 0; i < count; i++) {
        dataset->samples[i] = strdup(utf8_chars[i]);
    }

    return dataset;
}

void dataset_free(Dataset* dataset) {
    if (dataset) {
        if (dataset->samples) {
            for (size_t i = 0; i < dataset->length; i++) {
                free(dataset->samples[i]);
            }
            free(dataset->samples);
        }
        free(dataset);
    }
}

void dataset_print(Dataset* dataset) {
    for (size_t i = 0; i < dataset->length; i++) {
        printf("index=%zu, char=%s\n", i, dataset->samples[i]);
    }
}

int main(void) {
    setlocale(LC_ALL, "en_US.UTF-8");

    Dataset* dataset = dataset_create();
    if (!dataset) {
        printf("Failed to create dataset.\n");
        return EXIT_FAILURE;
    }

    printf("Generated UTF-8 dataset:\n");
    dataset_print(dataset);

    dataset_free(dataset);

    return EXIT_SUCCESS;
}
