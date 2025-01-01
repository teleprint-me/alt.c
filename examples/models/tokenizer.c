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

#include "algorithm/hash.h"

typedef struct VocabularyEntry {
    char* word;       // Space-separated symbols
    int* frequency;   // Pointer to frequency count
} VocabularyEntry;

VocabularyEntry* create_vocab_entry(const char* word, int frequency) {
    VocabularyEntry* entry = (VocabularyEntry*) malloc(sizeof(VocabularyEntry));
    if (!entry) {
        fprintf(stderr, "Error: Failed to allocate memory for VocabularyEntry.\n");
        return NULL;
    }

    // Allocate and copy the word
    entry->word = strdup(word);
    if (!entry->word) {
        fprintf(stderr, "Error: Failed to allocate memory for word.\n");
        free(entry);
        return NULL;
    }

    // Allocate and initialize frequency
    entry->frequency = (int*) malloc(sizeof(int));
    if (!entry->frequency) {
        fprintf(stderr, "Error: Failed to allocate memory for frequency.\n");
        free(entry->word);
        free(entry);
        return NULL;
    }
    *(entry->frequency) = frequency;

    return entry;
}

void free_vocab_entry(VocabularyEntry* entry) {
    if (entry) {
        if (entry->word) {
            free(entry->word);
        }
        if (entry->frequency) {
            free(entry->frequency);
        }
        free(entry);
    }
}

// Example vocabulary
HashTable* create_vocab(void) {
    HashTable* vocab = hash_create_table(16, HASH_TYPE_STRING);

    VocabularyEntry* entry = create_vocab_entry("l o w </w>", 5);
    if (hash_insert(vocab, entry->word, entry) != HASH_SUCCESS) {
        fprintf(stderr, "Error: Failed to insert 'l o w </w>' into vocab.\n");
        free_vocab_entry(entry);
    }

    entry = create_vocab_entry("l o w e r </w>", 2);
    if (hash_insert(vocab, entry->word, entry) != HASH_SUCCESS) {
        fprintf(stderr, "Error: Failed to insert 'l o w e r </w>' into vocab.\n");
        free_vocab_entry(entry);
    }

    entry = create_vocab_entry("n e w e s t </w>", 6);
    if (hash_insert(vocab, entry->word, entry) != HASH_SUCCESS) {
        fprintf(stderr, "Error: Failed to insert 'n e w e s t </w>' into vocab.\n");
        free_vocab_entry(entry);
    }

    entry = create_vocab_entry("w i d e s t </w>", 3);
    if (hash_insert(vocab, entry->word, entry) != HASH_SUCCESS) {
        fprintf(stderr, "Error: Failed to insert 'w i d e s t </w>' into vocab.\n");
        free_vocab_entry(entry);
    }

    return vocab;
}

void free_vocab(HashTable* vocab) {
    if (vocab) {
        for (uint64_t i = 0; i < vocab->size; i++) {
            HashEntry* entry = &vocab->entries[i];
            if (entry->key) {
                free(entry->key); // Free the key
                entry->key = NULL;
            }

            if (entry->value) {
                free_vocab_entry(entry->value); // Free the VocabularyEntry
                entry->value = NULL;
            }
        }
        hash_free_table(vocab); // Free the hash table itself
    }
}

HashTable* get_stats(HashTable* vocab) {
    HashTable* stats = hash_create_table(64, HASH_TYPE_STRING);

    for (uint64_t i = 0; i < vocab->size; ++i) {
        HashEntry* entry = &vocab->entries[i];
        if (entry->key) {
            VocabularyEntry* vocab_entry = (VocabularyEntry*) entry->value;

            // Split the word into symbols
            char* word = strdup(vocab_entry->word);
            char* token = strtok(word, " ");
            char* prev = token;
            while ((token = strtok(NULL, " ")) != NULL) {
                // Create the symbol pair
                char pair[64];
                snprintf(pair, sizeof(pair), "%s %s", prev, token);

                // Update frequency in stats
                int* frequency = (int*) hash_search(stats, pair);
                if (frequency) { // each frequency has its own memory now
                    (*frequency) += *vocab_entry->frequency; // points to unique frequency
                } else {
                    int* freq_ptr = malloc(sizeof(int));
                    *freq_ptr = *vocab_entry->frequency;
                    hash_insert(stats, strdup(pair), frequency); // pass freq by ref
                }

                prev = token;
            }
            free(word); // free the word so we don't have dangling pointers
        }
    }

    return stats;
}

// stats shares pointers with the vocab, but allocates keys.
void free_stats(HashTable* stats) {
    if (stats) {
        // Iterate over the entire hash table size
        for (uint64_t i = 0; i < stats->size; i++) {
            HashEntry* entry = &stats->entries[i];
            if (entry->key) {
                // Free the key (owned by the caller)
                free(entry->key);
                entry->key = NULL;
            }
            if (entry->value) {
                free(entry->value);
                entry->value = NULL;
            }
        }

        // Free the hash table (entries are owned by the hash table)
        hash_free_table(stats);
    }
}

void merge_vocab(HashTable* vocab, const char* pair) {
    char pattern[64];
    snprintf(pattern, sizeof(pattern), "(?<!\\S)%s(?!\\S)", pair);

    for (uint64_t i = 0; i < vocab->size; ++i) {
        HashEntry* entry = &vocab->entries[i];
        if (entry->key) {
            VocabularyEntry* vocab_entry = (VocabularyEntry*)entry->value;

            // Replace the pair in the word
            char* new_word = malloc(strlen(vocab_entry->word) + 1);
            if (!new_word) {
                fprintf(stderr, "Memory allocation failed in merge_vocab.\n");
                exit(EXIT_FAILURE);
            }
            strcpy(new_word, vocab_entry->word);

            // Replace pattern with merged token
            char* pos = strstr(new_word, pair);
            if (pos) {
                size_t len = strlen(pair);
                memmove(pos, pos + len, strlen(pos + len) + 1);
                memmove(pos, pair, len - 1); // Add merged symbol
            }

            // Update vocab entry safely
            char* old_word = vocab_entry->word; // Keep track of the old word
            free(old_word); // Free the old word AFTER updating the pointer
            vocab_entry->word = strdup(new_word);
        }
    }
}

int main() {
    // Create initial vocabulary
    HashTable* vocab = create_vocab();

    for (int i = 0; i < 10; ++i) {
        // Get symbol pair frequencies
        HashTable* stats = get_stats(vocab);

        // Find the most frequent pair
        char* best_pair = NULL;
        int max_freq = 0;

        for (uint64_t j = 0; j < stats->size; ++j) {
            HashEntry* entry = &stats->entries[j];
            if (entry->key) {
                int* freq = (int*) entry->value;
                if (*freq > max_freq) {
                    max_freq = *freq;
                    best_pair = entry->key;
                }
            }
        }

        if (!best_pair) {
            break;
        }

        // Merge the most frequent pair
        merge_vocab(vocab, best_pair);

        printf("Merged: %s\n", best_pair);

        // Cleanup stats
        free_stats(stats);
    }

    // Print final vocabulary
    printf("Final Vocabulary:\n");
    for (uint64_t i = 0; i < vocab->size; ++i) {
        HashEntry* entry = &vocab->entries[i];
        if (entry->key) {
            VocabularyEntry* vocab_entry = (VocabularyEntry*) entry->value;
            printf("%s: %d\n", vocab_entry->word, vocab_entry->frequency);
        }
    }

    free_vocab(vocab);

    return 0;
}
