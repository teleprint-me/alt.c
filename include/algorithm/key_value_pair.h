/**
 * @file include/algorithm/key_value_pair.h
 * @brief Generic Key-Value Pair Interface
 *
 * @details
 * This interface defines a flexible structure for managing key-value pairs, along with
 * optional allocators for custom memory management. It is designed for use in a variety
 * of data structures (e.g., hash tables, binary trees) and supports user-defined key and
 * value comparison functions.
 *
 * @warning Thread-safety is not guaranteed. Users are responsible for ensuring thread
 * safety if required.
 *
 * @note The design is still evolving. This is a conceptual sketch for a flexible key-value
 * pair abstraction to simplify future generalizations across data structures.
 */

#ifndef ALT_KEY_VALUE_PAIR_H
#define ALT_KEY_VALUE_PAIR_H

#include <stddef.h>

/**
 * @brief Function pointer for comparing two keys.
 *
 * @param key_a Pointer to the first key.
 * @param key_b Pointer to the second key.
 * @return 0 if equal, < 0 if key_a < key_b, > 0 if key_a > key_b.
 */
typedef int (*KVPairKeyCompare)(const void* key_a, const void* key_b);

/**
 * @brief Function pointer for comparing two values.
 *
 * @param value_a Pointer to the first value.
 * @param value_b Pointer to the second value.
 * @return 0 if equal, < 0 if value_a < value_b, > 0 if value_a > value_b.
 */
typedef int (*KVPairValueCompare)(const void* value_a, const void* value_b);

/**
 * @struct KeyValuePair
 * @brief Represents a single key-value pair.
 *
 * This structure associates a key with a value. The key and value are treated
 * as opaque pointers, allowing flexibility in the types of objects stored.
 */
typedef struct KeyValuePair {
    void* key; /**< Pointer to the key object. */
    void* value; /**< Pointer to the value object. */
} KeyValuePair;

/**
 * @struct KVPairAllocator
 * @brief Optional memory management hooks for keys and values.
 *
 * This structure provides customizable allocators and deallocators for the key
 * and value objects, enabling users to manage memory for complex or variable-sized
 * objects.
 */
typedef struct KVPairAllocator {
    // Memory management for keys
    void* (*allocate_key)(void* object, size_t size);
    void (*deallocate_key)(void* object);
    // Memory management for values
    void* (*allocate_value)(void* object, size_t size);
    void (*deallocate_value)(void* object);
} KVPairAllocator;

/**
 * @brief Creates a new key-value pair.
 *
 * @param allocator Optional allocator for managing key and value memory.
 *                  If NULL, the caller must manage memory for the key and value.
 * @return Pointer to a new KeyValuePair, or NULL if allocation fails.
 */
KeyValuePair* kv_pair_create(KVPairAllocator* allocator);

/**
 * @brief Destroys a key-value pair and releases associated memory.
 *
 * @param allocator Optional allocator for freeing key and value memory.
 *                  If NULL, only the KeyValuePair structure itself is freed.
 * @param pair Pointer to the KeyValuePair to destroy.
 */
void kv_pair_destroy(KVPairAllocator* allocator, KeyValuePair* pair);

/**
 * @brief Retrieves the key from a key-value pair.
 *
 * @param pair Pointer to the KeyValuePair.
 * @return Pointer to the key object, or NULL if the pair is NULL.
 */
void* kv_pair_get_key(const KeyValuePair* pair);

/**
 * @brief Retrieves the value from a key-value pair.
 *
 * @param pair Pointer to the KeyValuePair.
 * @return Pointer to the value object, or NULL if the pair is NULL.
 */
void* kv_pair_get_value(const KeyValuePair* pair);

/**
 * @brief Sets the key of a key-value pair.
 *
 * @param pair Pointer to the KeyValuePair.
 * @param key Pointer to the new key object.
 */
void kv_pair_set_key(KeyValuePair* pair, void* key);

/**
 * @brief Sets the value of a key-value pair.
 *
 * @param pair Pointer to the KeyValuePair.
 * @param value Pointer to the new value object.
 */
void kv_pair_set_value(KeyValuePair* pair, void* value);

/**
 * @brief Default comparison function for integer keys.
 *
 * @param key_a Pointer to the first key.
 * @param key_b Pointer to the second key.
 * @return 0 if equal, < 0 if key_a < key_b, > 0 if key_a > key_b.
 */
int kv_pair_key_compare_int32(const void* key_a, const void* key_b);

/**
 * @brief Default comparison function for string keys.
 *
 * @param key_a Pointer to the first key.
 * @param key_b Pointer to the second key.
 * @return 0 if equal, < 0 if key_a < key_b, > 0 if key_a > key_b.
 */
int kv_pair_key_compare_string(const void* key_a, const void* key_b);

#endif // ALT_KEY_VALUE_PAIR_H
