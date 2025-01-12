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

BinaryTreeState binary_tree_insert(BinaryTree* tree, BinaryTreePair* pair) {
    if (NULL == tree || NULL == pair) {
        LOG_ERROR("%s: Tree or pair is NULL\n", __func__);
        return BINARY_TREE_ERROR;
    }
    if (0 != pthread_rwlock_wrlock(&tree->rwlock)) {
        LOG_ERROR("%s: Failed to acquire write lock\n", __func__);
        return BINARY_TREE_LOCK_ERROR;
    }

    BinaryTreeNode* new_node = binary_tree_node_create(pair);
    if (NULL == new_node) {
        LOG_ERROR("%s: Failed to create a new BinaryTreeNode\n", __func__);
        return BINARY_TREE_MEMORY_ERROR;
    }

    BinaryTreeNode* parent = NULL;
    BinaryTreeNode* current = tree->root;
    while (NULL != current) {
        parent = current;
        if (0 > tree->compare(pair->key, current->pair->key)) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    new_node->parent = parent;
    if (NULL == parent) {
        // The tree was empty
        tree->root = new_node;
    } else if (0 > tree->compare(pair->key, parent->pair->key)) {
        parent->left = new_node;
    } else {
        parent->right = new_node;
    }

    if(0 != pthread_rwlock_unlock(&tree->rwlock)) {
        LOG_ERROR("%s: Failed to unlock rwlock\n", __func__);
        return BINARY_TREE_LOCK_ERROR;
    }
    return BINARY_TREE_SUCCESS;
}

void binary_tree_delete(BinaryTree* tree, BinaryTreePair* pair) {
    if (NULL == tree || NULL == pair) {
        LOG_ERROR("%s: Tree or pair is NULL\n", __func__);
        return BINARY_TREE_ERROR;
    }
    if (0 != pthread_rwlock_wrlock(&tree->rwlock)) {
        LOG_ERROR("%s: Failed to lock rwlock\n", __func__);
        return BINARY_TREE_LOCK_ERROR;
    }

    BinaryTreeNode* node_to_delete = binary_tree_find_node(tree, pair->key);
    if (NULL == node_to_delete) {
        LOG_ERROR("%s: Node with the given key not found\n", __func__);
        pthread_rwlock_unlock(&tree->rwlock);
        return;
    }

    if (NULL == node_to_delete->left) {
        binary_tree_transplant(tree, node_to_delete, node_to_delete->right);
    } else if (NULL == node_to_delete->right) {
        binary_tree_transplant(tree, node_to_delete, node_to_delete->left);
    } else {
        BinaryTreeNode* successor = binary_tree_find_minimum(tree, node_to_delete->right->pair->key);
        if (successor->parent != node_to_delete) {
            binary_tree_transplant(tree, successor, successor->right);
            successor->right = node_to_delete->right;
            if (NULL != successor->right) {
                successor->right->parent = successor;
            }
        }
        binary_tree_transplant(tree, node_to_delete, successor);
        successor->left = node_to_delete->left;
        if (NULL != successor->left) {
            successor->left->parent = successor;
        }
    }

    binary_tree_node_free(node_to_delete);
    if (0 != pthread_rwlock_unlock(&tree->rwlock)) {
        LOG_ERROR("%s: Failed to unlock rwlock\n", __func__);
        return BINARY_TREE_LOCK_ERROR;
    }
    return BINARY_TREE_SUCCESS;
}

BinaryTreeState binary_tree_transplant(BinaryTree* tree, BinaryTreeNode* old_node, BinaryTreeNode* new_node) {
    if (NULL == tree || NULL == old_node) {
        LOG_ERROR("%s: Tree or old_node is NULL\n", __func__);
        return BINARY_TREE_ERROR;
    }

    if (NULL == old_node->parent) {
        tree->root = new_node;
    } else if (old_node == old_node->parent->left) {
        old_node->parent->left = new_node;
    } else {
        old_node->parent->right = new_node;
    }

    if (NULL != new_node) {
        new_node->parent = old_node->parent;
    }

    return BINARY_TREE_SUCCESS;
}
