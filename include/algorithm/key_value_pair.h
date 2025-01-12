/**
 * @file include/algorithm/key_value_pair.h
 * @brief Key-value pair
 */

#ifndef ALT_KEY_VALUE_PAIR_H
#define ALT_KEY_VALUE_PAIR_H

#include <stddef.h>

typedef struct KeyValuePair {
    void* key;
    void* value;
} KeyValuePair;

typedef struct KeyValuePairAllocator {
    void* (*allocator)(void* object, size_t size);
    void (*deallocator)(void* object);
} KeyValuePairAllocator;

KeyValuePair* kv_pair_create(KeyValuePairAllocator* allocator);
void kv_pair_destroy(KeyValuePair* pair);

void kv_pair_set_key(KeyValuePair* pair, void* key);
void kv_pair_set_value(KeyValuePair* pair, void* value);
void kv_pair_get_key(KeyValuePair* pair, void** key);
void kv_pair_get_value(KeyValuePair* pair, void** value);

#endif // ALT_KEY_VALUE_PAIR_H
