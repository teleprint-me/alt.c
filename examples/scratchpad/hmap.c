/**
 * @file examples/scratchpad/hmap.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uthash.h>

// Define the structure for a user
typedef struct {
    int id; // Key for ID-based hash table
    char name[50]; // Key for name-based hash table
    UT_hash_handle hh_id; // Handle for ID hash table
    UT_hash_handle hh_name; // Handle for name hash table
} User;

// Global pointers to the hash tables
User* users_by_id = NULL; // Hash table for lookups by ID
User* users_by_name = NULL; // Hash table for lookups by name

// Function to add a user
void add_user(int id, const char* name) {
    User* user = malloc(sizeof(User));
    if (user == NULL) {
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    user->id = id;
    snprintf(user->name, sizeof(user->name), "%s", name);

    // Add to both hash tables
    HASH_ADD(hh_id, users_by_id, id, sizeof(int), user); // Add by ID
    HASH_ADD(hh_name, users_by_name, name, strlen(name), user); // Add by name
}

// Function to find a user by ID
User* find_user_by_id(int id) {
    User* user = NULL;
    HASH_FIND(hh_id, users_by_id, &id, sizeof(int), user); // Find in ID hash table
    return user;
}

// Function to find a user by name
User* find_user_by_name(const char* name) {
    User* user = NULL;
    HASH_FIND(hh_name, users_by_name, name, strlen(name), user); // Find in name hash table
    return user;
}

// Function to delete a user by ID
void delete_user(int id) {
    User *user = find_user_by_id(id);
    if (user) {
        // Use HASH_DELETE for custom handles
        HASH_DELETE(hh_id, users_by_id, user);   // Delete from ID hash table
        HASH_DELETE(hh_name, users_by_name, user); // Delete from name hash table
        free(user);                              // Free memory
    }
}

// Function to print all users (from the ID hash table)
void print_users(void) {
    User *current, *tmp;
    HASH_ITER(hh_id, users_by_id, current, tmp) { // Iterate through ID hash table
        printf("ID: %d, Name: %s\n", current->id, current->name);
    }
}

// Function to free all users
void free_users(void) {
    User *current, *tmp;

    // Delete all items from users_by_id (ensures all users are deleted once)
    HASH_ITER(hh_id, users_by_id, current, tmp) {
        HASH_DELETE(hh_id, users_by_id, current);   // Delete from ID hash table
        HASH_DELETE(hh_name, users_by_name, current); // Delete from name hash table
        free(current);                             // Free memory
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

    // Find a user by ID
    int id_to_find = 2;
    User* user = find_user_by_id(id_to_find);
    if (user) {
        printf("Found by ID: ID=%d, Name=%s\n", user->id, user->name);
    } else {
        printf("User with ID %d not found.\n", id_to_find);
    }

    // Find a user by name
    const char* name_to_find = "Alice";
    user = find_user_by_name(name_to_find);
    if (user) {
        printf("Found by Name: ID=%d, Name=%s\n", user->id, user->name);
    } else {
        printf("User with name '%s' not found.\n", name_to_find);
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
