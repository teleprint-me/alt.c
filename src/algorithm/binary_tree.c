/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/algorithm/binary_tree.c
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

#include "algorithm/binary_tree.h"
#include "interface/logger.h"

// ---------------------- Life-cycle functions ----------------------

// Create a new pair
BinaryTreePair* binary_tree_pair_create(void* key, void* value) {
    BinaryTreePair* pair = malloc(sizeof(BinaryTreePair));
    if (!pair) {
        LOG_ERROR("%s: Failed to allocate memory for pair\n", __func__);
        return NULL;
    }
    pair->key = key;
    pair->value = value;
    return pair;
}

// @note This function does not free the key or value.
// The caller must manage the memory separately.
void binary_tree_pair_free(BinaryTreePair* pair) {
    if (pair) {
        free(pair);
    }
}

// Create a new node
BinaryTreeNode* binary_tree_node_create(BinaryTreePair* pair) {
    BinaryTreeNode* node = malloc(sizeof(BinaryTreeNode));
    if (!node) {
        LOG_ERROR("%s: Failed to allocate memory for node\n", __func__);
        return NULL;
    }
    node->pair = pair;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    return node;
}

void binary_tree_node_free(BinaryTreeNode* node) {
    if (node) {
        if (node->pair) {
            binary_tree_pair_free(node->pair);
        }
        free(node);
    }
}

// Create a new tree
BinaryTree* binary_tree_create(void) {
    BinaryTree* tree = malloc(sizeof(BinaryTree));
    if (!tree) {
        LOG_ERROR("%s: Failed to allocate memory for tree\n", __func__);
        return NULL;
    }
    tree->root = NULL;
    tree->compare = NULL;
    // DO NOT REINITIALIZE THE RWLOCK!
    if (0 != pthread_rwlock_init(&tree->rwlock, NULL)) {
        LOG_ERROR("%s: Failed to initialize rwlock\n", __func__);
        free(tree);
        return NULL;
    }
    return tree;
}

void binary_tree_free(BinaryTree* tree) {
    if (tree) {
        while (tree->root) {
            binary_tree_delete(tree, tree->root);
        }
        pthread_rwlock_destroy(&tree->rwlock);
        free(tree);
    }
}
