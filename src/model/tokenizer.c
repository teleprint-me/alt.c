/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/model/tokenizer.h
 * @brief Interface for string and token manipulation supporting common ASCII and UTF-8 operations.
 * @note The tokenizer types are implementation specific. This API simply provides them. Model
 * implementation details are left up to the user. The models are converted from SentencePiece
 * binary format to ALT binary format. The tokenizer types here follow the expected file format
 * according to the specification.
 * @warning The implementation should expect BPE or BPE-like models and does not support unigram.
 *
 * This module is responsible for converting and handling user inputs and outputs.
 * - Normalization (NFKC Unicode)
 * - Pre-tokenization
 * - Merge pairs
 * - Merge frequencies
 * - Substitutions
 * - Post-processing
 * - Training
 * - Calculating frequency and score probabilities
 * - Token to id conversion
 * - Id to token conversion
 */

#include "interface/logger.h"

#include "model/tokenizer.h"

VocabularyEntry* create_vocab_entry(const char* word, int frequency) {
    VocabularyEntry* entry = (VocabularyEntry*) malloc(sizeof(VocabularyEntry));
    if (!entry) {
        LOG_ERROR("%s: Failed to allocate memory for VocabularyEntry.\n");
        return NULL;
    }

    // Allocate and copy the word
    entry->word = strdup(word);
    if (!entry->word) {
        LOG_ERROR("%s: Failed to allocate memory for word.\n");
        free(entry);
        return NULL;
    }

    // Allocate and initialize frequency
    entry->frequency = (int*) malloc(sizeof(int));
    if (!entry->frequency) {
        LOG_ERROR("%s: Failed to allocate memory for frequency.\n");
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

HashTable* get_stats(HashTable* vocab) {
    HashTable* stats = hash_create_table(64, HASH_TYPE_STRING);

    for (uint64_t i = 0; i < vocab->size; ++i) {
        HashTableEntry* entry = &vocab->entries[i];
        if (entry->key) {
            VocabularyEntry* vocab_entry = (VocabularyEntry*) entry->value;

            // Split the word into symbols
            FlexString* split = flex_string_create_split(vocab_entry->word, " ");
            if (!split || split->length < 2) {
                flex_string_free(split); // No pairs possible, free and continue
                continue;
            }

            // Create pairs of adjacent symbols
            for (uint32_t j = 0; j < split->length - 1; ++j) {
                char pair[64];
                snprintf(pair, sizeof(pair), "%s %s", split->parts[j], split->parts[j + 1]);

                // Update frequency in stats
                int* frequency = (int*) hash_search(stats, pair);
                if (frequency) {
                    (*frequency) += *(vocab_entry->frequency);
                } else {
                    frequency = (int*) malloc(sizeof(int));
                    *frequency = *(vocab_entry->frequency);
                    hash_insert(stats, strdup(pair), frequency);
                }
            }

            flex_string_free(split);
        }
    }

    return stats;
}

// stats shares pointers with the vocab, but allocates keys.
void free_stats(HashTable* stats) {
    if (stats) {
        // Iterate over the entire hash table size
        for (uint64_t i = 0; i < stats->size; i++) {
            HashTableEntry* entry = &stats->entries[i];
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
        HashTableEntry* entry = &vocab->entries[i];
        if (entry->key) {
            VocabularyEntry* vocab_entry = (VocabularyEntry*) entry->value;

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

HashTable* create_byte_map(void) {
    // Create a hash table with a size of 256 to map 256 possible byte values
    HashTable* map = hash_create_table(256, HASH_TYPE_STRING);
    if (!map) {
        return NULL; // Handle memory allocation failure
    }

    // Populate the hash table with the tokens corresponding to each byte value
    for (unsigned int i = 0; i < 256; ++i) {
        char* token = byte_to_token(i);
        if (hash_insert(map, token, (void*) (uintptr_t) i) != HASH_SUCCESS) {
            free(token); // Handle insertion failure
            return NULL;
        }
        free(token); // Free the temporary token string
    }

    return map;
}

char* byte_to_token(unsigned char byte) {
    // Max value is 0xFF (255 = 2^(n-1))
    // if (byte > 0xFF) { // comparison is always true due to limited range of data type.
    //     return NULL;
    // }

    // Get the width of a byte token
    size_t length = strlen("<0xXX>") + 1;

    // Allocate space for the formatted string
    char* token = (char*) malloc(length * sizeof(char)); // Enough for "<0xXX>" format
    if (!token) {
        return NULL; // Handle memory allocation failure
    }

    // Format the byte as "<0xXX>"
    snprintf(token, 10, "<0x%02X>", byte);
    return token;
}

int token_to_byte(HashTable* byte_map, const char* token) {
    unsigned char* byte = (unsigned char*) hash_search(byte_map, token);
    if (!byte) {
        return -1; // Token not found
    }
    return (int) *byte; // Return the byte value
}
