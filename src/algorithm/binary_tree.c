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
#include <stdlib.h>

// ---------------------- Life-cycle functions ----------------------

BinaryTreePair* binary_tree_pair_create(void* key, void* value) {
    BinaryTreePair* pair = malloc(sizeof(BinaryTreePair));
    if (!pair) {
        LOG_ERROR("%s: Failed to allocate memory for BinaryTreePair\n", __func__);
        return NULL;
    }
    pair->key = key;
    pair->value = value;
    return pair;
}

void binary_tree_pair_free(BinaryTreePair* pair) {
    if (pair) {
        free(pair);
    }
}

BinaryTreeNode* binary_tree_node_create(BinaryTreePair* pair) {
    BinaryTreeNode* node = malloc(sizeof(BinaryTreeNode));
    if (!node) {
        LOG_ERROR("%s: Failed to allocate memory for BinaryTreeNode\n", __func__);
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

BinaryTree* binary_tree_create(BinaryTreePairCompare compare) {
    BinaryTree* tree = malloc(sizeof(BinaryTree));
    if (!tree) {
        LOG_ERROR("%s: Failed to allocate memory for BinaryTree\n", __func__);
        return NULL;
    }
    tree->root = NULL;
    tree->compare = (compare) ? compare : binary_tree_pair_int32_compare;

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
            binary_tree_delete(tree, tree->root); // Assuming this will handle node cleanup
        }
        if (0 != pthread_rwlock_destroy(&tree->rwlock)) {
            LOG_ERROR("%s: Failed to destroy rwlock\n", __func__);
        }
        free(tree);
    }
}

// ---------------------- Default comparison functions ----------------------

int binary_tree_pair_int32_compare(const void* key_a, const void* key_b) {
    if (!key_a || !key_b) {
        LOG_ERROR("%s: Null key provided for comparison\n", __func__);
        return 0;
    }
    int a = *(int*)key_a;
    int b = *(int*)key_b;
    return (a > b) - (a < b); // Returns -1, 0, or 1
}

int binary_tree_pair_string_compare(const void* key_a, const void* key_b) {
    if (!key_a || !key_b) {
        LOG_ERROR("%s: Null key provided for comparison\n", __func__);
        return 0;
    }
    const char* a = *(const char*)key_a;
    const char* b = *(const char*)key_b;
    return strcmp(a, b); // Returns -1, 0, or 1
}

// ---------------------- Insertion and Deletion Functions ----------------------

