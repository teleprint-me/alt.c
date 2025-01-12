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

#include "interface/logger.h"
#include "algorithm/binary_tree.h"

// ---------------------- Default comparison functions ----------------------

int binary_tree_node_compare_int32(const void* key_a, const void* key_b) {
    if (!key_a || !key_b) {
        LOG_ERROR("%s: Null key provided for comparison\n", __func__);
        return 0;
    }
    int a = *(int*) key_a;
    int b = *(int*) key_b;
    return (a > b) - (a < b); // Returns -1, 0, or 1
}

int binary_tree_node_compare_string(const void* key_a, const void* key_b) {
    if (!key_a || !key_b) {
        LOG_ERROR("%s: Null key provided for comparison\n", __func__);
        return 0;
    }
    const char* a = (const char*) key_a;
    const char* b = (const char*) key_b;
    return strcmp(a, b); // Returns -1, 0, or 1
}

// ---------------------- Create a key-value pair ----------------------

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

// ---------------------- Create a node ----------------------

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

BinaryTreeNode* binary_tree_node_create_from_pair(void* key, void* value) {
    BinaryTreePair* pair = malloc(sizeof(BinaryTreePair));
    if (!pair) {
        LOG_ERROR("%s: Failed to allocate memory for BinaryTreePair\n", __func__);
        return NULL;
    }
    pair->key = key;
    pair->value = value;
    return binary_tree_node_create(pair);
}

void binary_tree_node_free(BinaryTreeNode* node) {
    if (node) {
        binary_tree_pair_free(node->pair);
        free(node);
    }
}

// ---------------------- Create a tree ----------------------

BinaryTree* binary_tree_create(BinaryTreeKeyCompare compare) {
    BinaryTree* tree = malloc(sizeof(BinaryTree));
    if (!tree) {
        LOG_ERROR("%s: Failed to allocate memory for BinaryTree\n", __func__);
        return NULL;
    }
    tree->root = NULL;
    tree->compare = (compare) ? compare : binary_tree_node_compare_int32;

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

// ---------------------- Insertion and Deletion Functions ----------------------

BinaryTreeState binary_tree_insert(BinaryTree* tree, BinaryTreeNode* node) {
    if (!tree || !node) {
        LOG_ERROR("%s: Tree or node is NULL\n", __func__);
        return BINARY_TREE_ERROR;
    }

    if (0 != pthread_rwlock_wrlock(&tree->rwlock)) {
        LOG_ERROR("%s: Failed to acquire write lock\n", __func__);
        return BINARY_TREE_LOCK_ERROR;
    }

    BinaryTreeNode* parent = NULL;
    BinaryTreeNode* current = tree->root;

    // Traverse the tree to find the correct insertion point
    while (current) {
        parent = current;
        int cmp = tree->compare(node->pair->key, current->pair->key);
        if (0 == cmp) {
            LOG_ERROR("%s: Duplicate key detected\n", __func__);
            pthread_rwlock_unlock(&tree->rwlock);
            return BINARY_TREE_ERROR;
        } else if (0 > cmp) {
            current = current->left;
        } else {
            current = current->right;
        }
    }

    // Insert the node
    node->parent = parent;
    if (!parent) {
        tree->root = node; // Tree was empty
    } else if (0 > tree->compare(node->pair->key, parent->pair->key)) {
        parent->left = node;
    } else {
        parent->right = node;
    }

    if (0 != pthread_rwlock_unlock(&tree->rwlock)) {
        LOG_ERROR("%s: Failed to release write lock\n", __func__);
        return BINARY_TREE_LOCK_ERROR;
    }

    return BINARY_TREE_SUCCESS;
}

BinaryTreeState binary_tree_delete(BinaryTree* tree, BinaryTreeNode* node) {
    if (!tree || !node) {
        LOG_ERROR("%s: Tree or node is NULL\n", __func__);
        return BINARY_TREE_ERROR;
    }

    if (0 != pthread_rwlock_wrlock(&tree->rwlock)) {
        LOG_ERROR("%s: Failed to lock rwlock\n", __func__);
        return BINARY_TREE_LOCK_ERROR;
    }

    BinaryTreeNode* node_to_delete = binary_tree_search(tree, node->pair->key);
    if (!node_to_delete) {
        LOG_ERROR("%s: Node with the given key not found\n", __func__);
        pthread_rwlock_unlock(&tree->rwlock);
        return BINARY_TREE_NO_KEY;
    }

    // Case 1: Node has no left child
    if (!node_to_delete->left) {
        binary_tree_transplant(tree, node_to_delete, node_to_delete->right);
    }
    // Case 2: Node has no right child
    else if (!node_to_delete->right) {
        binary_tree_transplant(tree, node_to_delete, node_to_delete->left);
    }
    // Case 3: Node has two children
    else {
        BinaryTreeNode* successor = binary_tree_successor(node_to_delete);
        if (successor && successor->parent != node_to_delete) {
            binary_tree_transplant(tree, successor, successor->right);
            successor->right = node_to_delete->right;
            if (successor->right) {
                successor->right->parent = successor;
            }
        }
        binary_tree_transplant(tree, node_to_delete, successor);
        successor->left = node_to_delete->left;
        if (successor->left) {
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

BinaryTreeState
binary_tree_transplant(BinaryTree* tree, BinaryTreeNode* old_node, BinaryTreeNode* new_node) {
    if (!tree || !old_node) {
        LOG_ERROR("%s: Tree or old_node is NULL\n", __func__);
        return BINARY_TREE_ERROR;
    }

    // Case 1: old_node is the root
    if (!old_node->parent) {
        tree->root = new_node;
    }
    // Case 2: old_node is the left child
    else if (old_node == old_node->parent->left) {
        old_node->parent->left = new_node;
    }
    // Case 3: old_node is the right child
    else {
        old_node->parent->right = new_node;
    }

    // Update the parent pointer of the new_node
    if (new_node) {
        new_node->parent = old_node->parent;
    }

    return BINARY_TREE_SUCCESS;
}

// ---------------------- Search by key ----------------------

// Find a node in the tree (iterative search)
BinaryTreeNode* binary_tree_search(BinaryTree* tree, void* key) {
    // Handle edge cases
    if (!tree || !tree->root || !key) {
        LOG_ERROR("%s: Tree or key is NULL\n", __func__);
        return NULL;
    }

    BinaryTreeNode* node = tree->root;
    while (node) {
        int compare = tree->compare(node->pair->key, key);
        if (0 == compare) {
            return node; // Key found
        }
        node = (0 > compare) ? node->left : node->right;
    }

    // Key not found
    return NULL;
}

// ---------------------- Search by node ----------------------

BinaryTreeNode* binary_tree_minimum(BinaryTreeNode* node) {
    if (!node) {
        LOG_ERROR("%s: Node is NULL\n", __func__);
        return NULL;
    }

    // Do not overwrite the root node
    BinaryTreeNode* current = node;

    // Traverse to the leftmost node
    while (current->left) {
        current = current->left;
    }

    return current;
}

BinaryTreeNode* binary_tree_maximum(BinaryTreeNode* node) {
    if (!node) {
        LOG_ERROR("%s: Node is NULL\n", __func__);
        return NULL;
    }

    // Do not overwrite the root node
    BinaryTreeNode* current = node;

    // Traverse to the rightmost node
    while (current->right) {
        current = current->right;
    }

    return current;
}

BinaryTreeNode* binary_tree_successor(BinaryTreeNode* node) {
    if (!node) {
        LOG_ERROR("%s: Node is NULL\n", __func__);
        return NULL;
    }

    // Case 1: If the right subtree exists, find the minimum in the right subtree
    if (node->right) {
        return binary_tree_minimum(node->right);
    }

    // Case 2: Walk up the tree to find the first ancestor where `node` is a left child
    BinaryTreeNode* successor = node->parent;
    while (successor && node == successor->right) {
        node = successor;
        successor = successor->parent;
    }

    return successor;
}

BinaryTreeNode* binary_tree_predecessor(BinaryTreeNode* node) {
    if (!node) {
        LOG_ERROR("%s: Node is NULL\n", __func__);
        return NULL;
    }

    // Case 1: If the left subtree exists, find the maximum in the left subtree
    if (node->left) {
        return binary_tree_maximum(node->left);
    }

    // Case 2: Walk up the tree to find the first ancestor where `node` is a right child
    BinaryTreeNode* predecessor = node->parent;
    while (predecessor && node == predecessor->left) {
        node = predecessor;
        predecessor = predecessor->parent;
    }

    return predecessor;
}

// ---------------------- Walk the tree ----------------------

void binary_tree_inorder_walk(BinaryTreeNode* node, BinaryTreeNodeCallback callback) {
    if (!node) {
        return; // Base case: empty tree
    }

    // Traverse the left subtree
    binary_tree_inorder_walk(node->left, callback);

    // Apply the callback to the current node
    callback(node);

    // Traverse the right subtree
    binary_tree_inorder_walk(node->right, callback);
}

// ---------------------- Tree walking utilities ----------------------

void print_node_int32(BinaryTreeNode* node) {
    printf("key=%d\n", *(int*)(node->pair->key));
}

void print_node_string(BinaryTreeNode* node) {
    printf("key=%s\n", (char*)(node->pair->key));
}
