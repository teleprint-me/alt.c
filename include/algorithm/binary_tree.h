/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/binary_tree.h
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

// Key-value pair for a node in the tree
typedef struct BinaryTreePair {
    void* key; // The key of the node
    void* value; // The value of the node
} BinaryTreePair;

// Function pointer to compair pairs
typedef int (*BinaryTreePairCompare)(const void* key_a, const void* key_b);

// Node in the tree
typedef struct BinaryTreeNode {
    BinaryTreePair* pair;
    struct BinaryTreeNode* left;
    struct BinaryTreeNode* right;
    struct BinaryTreeNode* parent;
} BinaryTreeNode;

// Function pointer to process each node
typedef void (*BinaryTreeNodeCallback)(BinaryTreeNode* node);

// Tree allows reads, but locks on writes
typedef struct BinaryTree {
    BinaryTreeNode* root;
    BinaryTreePairCompare compare;
    pthread_rwlock_t rwlock; // _POSIX_C_SOURCE requires greater than or equal to 200112L.
} BinaryTree;

// ---------------------- Life-cycle functions ----------------------

// Create a new pair
BinaryTreePair* binary_tree_pair_create(void* key, void* value);
// @note This function does not free the key or value.
// The caller must manage the memory separately.
void binary_tree_pair_free(BinaryTreePair* pair);

// Create a new node
BinaryTreeNode* binary_tree_node_create(BinaryTreePair* pair);
void binary_tree_node_free(BinaryTreeNode* node);

// Create a new tree
BinaryTree* binary_tree_create(void);
void binary_tree_free(BinaryTree* tree);

// ---------------------- Insertion and Deletion Functions ----------------------

// Insert a new node into the tree
void binary_tree_insert(BinaryTree* tree, BinaryTreePair* pair);

// Delete a node from the tree
void binary_tree_delete(BinaryTree* tree, BinaryTreePair* pair);

// Transplant a node in the tree with a new node
void binary_tree_transplant(BinaryTree* tree, BinaryTreePair* old_pair, BinaryTreePair* new_pair);

// ---------------------- Searching Functions ----------------------

// Find a node in the tree
BinaryTreeNode* binary_tree_find_node(BinaryTree* tree, void* key);

// Find the minimum key in the tree where x is a node in the tree.
BinaryTreeNode* binary_tree_find_minimum(BinaryTree* tree, void* key);

// Find the maximum key in the tree where x is a node in the tree.
BinaryTreeNode* binary_tree_find_maximum(BinaryTree* tree, void* key);

// Find the successor of a node in the tree
BinaryTreeNode* binary_tree_find_successor(BinaryTree* tree, void* key);

// Find the predecessor of a node in the tree
BinaryTreeNode* binary_tree_find_predecessor(BinaryTree* tree, void* key);

// ---------------------- Walking the tree ----------------------

// Traverse the tree in order
void binary_tree_inorder_walk(BinaryTree* tree, BinaryTreeNodeCallback callback);

// Perform a pre-order traversal of the tree
void binary_tree_preorder_walk(BinaryTree* tree, BinaryTreeNodeCallback callback);

// Perform a post-order traversal of the tree
void binary_tree_postorder_walk(BinaryTree* tree, BinaryTreeNodeCallback callback);

#endif // BINARY_TREE_H
