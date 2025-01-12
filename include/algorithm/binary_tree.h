/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/algorithm/binary_tree.h
 * @brief Binary tree data structure.
 *
 * A binary search tree implementation with typical traversal methods and
 * key-value management.
 *
 * Binary-search tree properties:
 * - Each node has a key and value, and pointers to left, right, and parent nodes.
 * - If a child or parent is missing, the pointer is NULL.
 * - The root node has no parent (parent is NULL).
 * - Left child keys are <= parent key; right child keys are >= parent key.
 * - Supports in-order, pre-order, and post-order tree walks.
 */

#ifndef BINARY_TREE_H
#define BINARY_TREE_H

#include <pthread.h>
#include <stdlib.h>

// ---------------------- Enumerations ----------------------

typedef enum BinaryTreeState {
    BINARY_TREE_SUCCESS, // Action was successful
    BINARY_TREE_ERROR, // General failure
    BINARY_TREE_NO_PARENT, // Node has no parent
    BINARY_TREE_NO_CHILD, // Node has no child
    BINARY_TREE_NO_KEY, // Key not found in tree
    BINARY_TREE_NO_VALUE, // Value not found in tree
    BINARY_TREE_MEMORY_ERROR, // Memory allocation failed
    BINARY_TREE_LOCK_ERROR // Thread lock operation failed
} BinaryTreeState;

// ---------------------- Structures ----------------------

// Key-value pair for a node in the tree
typedef struct BinaryTreePair {
    void* key; // The key of the node
    void* value; // The value of the node
} BinaryTreePair;

// Node in the tree
typedef struct BinaryTreeNode {
    BinaryTreePair* pair; // The key-value pair
    struct BinaryTreeNode* left; // The left child of the node
    struct BinaryTreeNode* right; // The right child of the node
    struct BinaryTreeNode* parent; // The parent of the node
} BinaryTreeNode;

// Tree allows reads, but locks on writes
typedef struct BinaryTree {
    BinaryTreeNode* root; // The root node of the tree
    BinaryTreeKeyCompare compare; // The function used to compare keys
    pthread_rwlock_t rwlock; // _POSIX_C_SOURCE requires greater than or equal to 200112L.
} BinaryTree;

// ---------------------- Function pointers ----------------------

// Function pointer to process each node
typedef BinaryTreeState (*BinaryTreeNodeCallback)(BinaryTreeNode* node);

/**
 * @brief Compares two keys.
 *
 * @param key_a Pointer to the first key.
 * @param key_b Pointer to the second key.
 * @return -1 if key_a < key_b, 0 if key_a == key_b, 1 if key_a > key_b.
 */
typedef int (*BinaryTreeKeyCompare)(const void* key_a, const void* key_b);

// ---------------------- Default comparison functions ----------------------

int binary_tree_node_compare_int32(const void* key_a, const void* key_b);
int binary_tree_node_compare_string(const void* key_a, const void* key_b);

// ---------------------- Create a key-value pair ----------------------

/// @note This function does not allocate the key or value. The caller must manage the memory
/// separately.
BinaryTreePair* binary_tree_pair_create(void* key, void* value);
/// @note This function does not free the key or value. The caller must manage the memory
/// separately.
void binary_tree_pair_free(BinaryTreePair* pair);

// ---------------------- Create a node ----------------------

// Create a new node
BinaryTreeNode* binary_tree_node_create(BinaryTreePair* pair);
/// @note This function does not allocate the key or value. The caller must manage the memory
/// separately.
BinaryTreeNode* binary_tree_node_create_from_pair(void* key, void* value);
// Free a node
void binary_tree_node_free(BinaryTreeNode* node);

// ---------------------- Create a tree ----------------------

// Create a new tree
BinaryTree* binary_tree_create(BinaryTreeKeyCompare compare);
// Free the tree
void binary_tree_free(BinaryTree* tree);

// ---------------------- Insertion and Deletion Functions ----------------------

// Insert a new node into the tree
BinaryTreeState binary_tree_insert(BinaryTree* tree, BinaryTreeNode* node);

// Delete a node from the tree
BinaryTreeState binary_tree_delete(BinaryTree* tree, BinaryTreeNode* node);

// Transplant a node in the tree with a new node
BinaryTreeState
binary_tree_transplant(BinaryTree* tree, BinaryTreeNode* old_node, BinaryTreeNode* new_node);

// ---------------------- Search by key ----------------------

// Find a node in the tree (iterative search)
BinaryTreeNode* binary_tree_search(BinaryTree* tree, void* key);

// ---------------------- Search by node ----------------------

// Find the minimum key in the tree where x is a node in the tree.
BinaryTreeNode* binary_tree_minimum(BinaryTreeNode* node);

// Find the maximum key in the tree where x is a node in the tree.
BinaryTreeNode* binary_tree_maximum(BinaryTreeNode* node);

// The successor is the node with the smallest key greater than the given node.
BinaryTreeNode* binary_tree_successor(BinaryTreeNode* node);

// The predecessor is the node with the largest key smaller than the given node.
BinaryTreeNode* binary_tree_predecessor(BinaryTreeNode* node);

// ---------------------- Walk the tree ----------------------

// Traverse the tree in order
void binary_tree_inorder_walk(BinaryTree* tree, BinaryTreeNodeCallback callback);

// Perform a pre-order traversal of the tree
void binary_tree_preorder_walk(BinaryTree* tree, BinaryTreeNodeCallback callback);

// Perform a post-order traversal of the tree
void binary_tree_postorder_walk(BinaryTree* tree, BinaryTreeNodeCallback callback);

#endif // BINARY_TREE_H
