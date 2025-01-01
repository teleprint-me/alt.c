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

#include "interface/flex_string.h"

#include "model/tokenizer.h"

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
            printf("%s: %d\n", vocab_entry->word, *vocab_entry->frequency);
        }
    }

    free_vocab(vocab);

    return 0;
}
