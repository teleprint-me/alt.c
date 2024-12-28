/**
 * @file examples/scratchpad/hmap.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <uthash.h>

// Define the structure for a user
typedef struct User {
    int id; // Key (unique identifier)
    char name[50]; // Value (name of the user)
    UT_hash_handle hh; // Makes this structure hashable
} User;

// Global pointer to the hash table
User* users = NULL;

// Function to add a user
void add_user(int id, const char* name) {
    User* user = malloc(sizeof(User));
    if (user == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    user->id = id;
    snprintf(user->name, sizeof(user->name), "%s", name);
    HASH_ADD_INT(users, id, user); // Add to the hash table
}

// Function to find a user by ID
User* find_user(int id) {
    User* user = NULL;
    HASH_FIND_INT(users, &id, user); // Find the user in the hash table
    return user;
}

// Function to delete a user by ID
void delete_user(int id) {
    User* user = find_user(id);
    if (user) {
        HASH_DEL(users, user); // Delete from the hash table
        free(user); // Free memory
    }
}

// Function to print all users
void print_users(void) {
    User *current, *tmp;
    HASH_ITER(hh, users, current, tmp) { // Iterate through all users
        printf("ID: %d, Name: %s\n", current->id, current->name);
    }
}

// Function to free the entire hash table
void free_users(void) {
    User *current, *tmp;
    HASH_ITER(hh, users, current, tmp) {
        HASH_DEL(users, current); // Delete from the hash table
        free(current); // Free memory
    }
}

int main() {
    // Add users
    add_user(1, "Alice");
    add_user(2, "Bob");
    add_user(3, "Charlie");

    // Print all users
    printf("All users:\n");
    print_users();

    // Find a user
    int id_to_find = 2;
    User* user = find_user(id_to_find);
    if (user) {
        printf("User found: ID=%d, Name=%s\n", user->id, user->name);
    } else {
        printf("User with ID %d not found.\n", id_to_find);
    }

    // Delete a user
    delete_user(2);

    // Print all users after deletion
    printf("All users after deletion:\n");
    print_users();

    // Free all users
    free_users();

    return 0;
}
