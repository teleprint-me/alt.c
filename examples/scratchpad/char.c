/**
 * @file examples/scratchpad/char.c
 *
 * @brief Prototype for auto-generating input characters.
 */

#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

typedef struct Dataset {
    size_t length;
    wchar_t* samples;
} Dataset;

// @ref https://stackoverflow.com/a/36315819/20035933
void print_progress(char* title, float percentage, uint32_t width, char ch) {
    char bar[width + 1];
    for (uint32_t i = 0; i < width; i++) {
        bar[i] = ch;
    }
    bar[width] = '\0'; // Null-terminate the bar for safety

    uint32_t progress = (uint32_t) (percentage * 100 + 0.5f); // Round percentage
    uint32_t left = (uint32_t) (percentage * width + 0.5f); // Round bar width
    uint32_t right = width - left;

    printf("\r%s: %3u%% [%.*s%*s]", title, progress, left, bar, right, "");
    fflush(stdout);
}

Dataset* dataset_create(size_t start, size_t end) {
    if (start >= end) {
        return NULL; // end must be greater than start
    }

    Dataset* dataset = (Dataset*) malloc(sizeof(Dataset));

    if (!dataset) {
        return NULL;
    }

    dataset->length = (end - start) + 1; // use the difference as the length
    dataset->samples = (wchar_t*) malloc(sizeof(wchar_t) * dataset->length);

    if (!dataset->samples) {
        free(dataset);
        return NULL;
    }

    // populate samples
    for (size_t i = 0, s = start; i < dataset->length && s < end; i++) {
        dataset->samples[i] = (wchar_t) (i + s);
    }

    return dataset;
}

void dataset_free(Dataset* dataset) {
    if (dataset) {
        if (dataset->samples) {
            free(dataset->samples);
        }
        free(dataset);
    }
}

uint32_t dataset_shuffle(Dataset* dataset) {
    uint32_t sample_count = 0; // Track swaps

    if (!dataset || !dataset->samples) {
        return 0;
    }

    for (uint32_t i = 0; i < dataset->length; i++, sample_count++) {
        float progress = (float) i / (float) (dataset->length - 1);
        print_progress("Shuffling", progress, 50, '#'); // Track progress

        uint32_t j = rand() % (dataset->length - i); // Pick a random index

        // Swap samples[i] and samples[j]
        wchar_t sample = dataset->samples[i];
        dataset->samples[i] = dataset->samples[j];
        dataset->samples[j] = sample;
    }
    printf("\n");

    return sample_count;
}

void dataset_print(Dataset* dataset) {
    for (uint32_t i = 0; i < dataset->length; i++) {
        wchar_t code = dataset->samples[i];
        printf("index=%d, code=%d, char=%lc\n", i, code, code);
    }
}

int main(void) {
    setlocale(LC_ALL, ""); /// @brief Enable wide characters

    size_t start = 32;
    size_t end = 255;

    Dataset* dataset = dataset_create(start, end);
    if (!dataset) {
        printf("Failed to create dataset.\n");
        return EXIT_FAILURE;
    }

    printf("Generated dataset with %zu wide characters:\n", dataset->length);
    dataset_print(dataset);

    uint32_t sample_count = dataset_shuffle(dataset);
    printf("Shuffled dataset with %u swaps:\n", sample_count);
    dataset_print(dataset);

    dataset_free(dataset);

    return EXIT_SUCCESS;
}
