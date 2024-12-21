/**
 * @file examples/models/tokenizer.c
 * @brief Create a simple BPE tokenizer.
 * @note This is just a simple sketch for now. Keeping the initial implementation as simple as
 * possible.
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define the vocabulary size
#define VOCAB_SIZE 9

// Vocabulary mapping (word -> index)
typedef struct {
    size_t index;
    const char* word;
} VocabEntry;

VocabEntry vocabulary[VOCAB_SIZE] = {
    {0, "the"  },
    {1, "quick"},
    {2, "brown"},
    {3, "fox"  },
    {4, "jumps"},
    {5, "over" },
    {6, "lazy" },
    {7, "dog"  },
    {8, "."    }
};

// Tokenize a string and convert to indices
size_t* tokenize(const char* sentence, size_t* out_size) {
    char* buffer = strdup(sentence); // Duplicate input to tokenize
    char* token = strtok(buffer, " "); // Split by space
    size_t capacity = 10; // Initial capacity for the result array
    size_t* indices = malloc(capacity * sizeof(size_t));
    *out_size = 0;

    while (token) {
        // Convert to lowercase for simplicity
        for (char* p = token; *p; ++p) {
            *p = tolower(*p);
        }

        // Remove punctuation at the end
        size_t len = strlen(token);
        if (ispunct(token[len - 1])) {
            token[len - 1] = '\0';
        }

        // Look up the token in the vocabulary
        size_t found = -1;
        for (size_t i = 0; i < VOCAB_SIZE; ++i) {
            if (strcmp(token, vocabulary[i].word) == 0) {
                found = vocabulary[i].index;
                break;
            }
        }

        if (found != -1lu) {
            if (*out_size >= capacity) {
                capacity *= 2;
                indices = realloc(indices, capacity * sizeof(size_t));
            }
            indices[*out_size] = found;
            (*out_size)++;
        }

        token = strtok(NULL, " ");
    }

    free(buffer);
    return indices;
}

int main() {
    const char* sentence = "The quick brown fox jumps over the lazy dog.";
    size_t token_count = 0;

    // Tokenize the sentence
    size_t* tokens = tokenize(sentence, &token_count);

    // Print the token indices
    printf("Token indices:\n");
    for (size_t i = 0; i < token_count; ++i) {
        printf("%zu: %s\n", tokens[i], vocabulary[i].word);
    }
    printf("\n");

    free(tokens);
    return 0;
}
