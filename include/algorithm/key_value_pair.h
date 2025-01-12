/**
 * @file include/algorithm/key_value_pair.h
 * @brief Key-value pair
 * @warning Thread-safety is not guaranteed. This is mostly due to the flexibility of the data
 * structure.
 */

#ifndef ALT_KEY_VALUE_PAIR_H
#define ALT_KEY_VALUE_PAIR_H

#include <stddef.h>

typedef struct KeyValuePair {
    void* key;
    void* value;
} KeyValuePair;

typedef struct KeyValuePairAllocator {
    // keys
    void* (*allocate_key)(void* object, size_t size);
    void (*deallocate_key)(void* object);
    // values
    void* (*allocate_value)(void* object, size_t size);
    void (*deallocate_value)(void* object);
} KeyValuePairAllocator;

// life-cycle
KeyValuePair* kv_pair_create(KeyValuePairAllocator* allocator);
void kv_pair_destroy(KeyValuePairAllocator* allocator, KeyValuePair* pair);

// getters
void* kv_pair_get_key(const KeyValuePair* pair);
void* kv_pair_get_value(const KeyValuePair* pair);

// setters
void kv_pair_set_key(KeyValuePair* pair, void* key);
void kv_pair_set_value(KeyValuePair* pair, void* value);

#endif // ALT_KEY_VALUE_PAIR_H
