/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/path.h
 *
 * @brief Path manipulation interface for C.
 */

#ifndef ALT_PATH_H
#define ALT_PATH_H

#include <asm/unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#define PATH_SEPARATOR_CHR '/'
#define PATH_SEPARATOR_STR "/"

typedef enum PathState {
    PATH_SUCCESS,
    PATH_ERROR,
    PATH_INVALID_ARGUMENT,
    PATH_PERMISSION_DENIED,
    PATH_NOT_FOUND,
    PATH_NOT_A_DIRECTORY,
    PATH_SYMLINK_LOOP,
    PATH_MEMORY_ALLOCATION,
    PATH_UNKNOWN
} PathState;

typedef enum {
    FILE_TYPE_UNKNOWN, // Unknown file type
    FILE_TYPE_REGULAR, // Regular file
    FILE_TYPE_DIRECTORY, // Directory
    FILE_TYPE_SYMLINK, // Symbolic link
    FILE_TYPE_BLOCK_DEVICE, // Block device
    FILE_TYPE_CHAR_DEVICE, // Character device
    FILE_TYPE_PIPE, // Named pipe (FIFO)
    FILE_TYPE_SOCKET // Socket
} PathType;

typedef struct {
    char* path; // Full path
    char* name; // Entry name (basename)
    char* parent; // Parent directory (dirname)
    PathType type; // File type (enum)
    off_t size; // File size
    ino_t inode; // Inode number
    uid_t uid; // Owner user ID
    gid_t gid; // Owner group ID
    time_t atime; // Last access time
    time_t mtime; // Last modification time
    time_t ctime; // Creation (or metadata change) time
} PathInfo;

// Represents directory entries from a traversal
typedef struct PathEntry {
    PathInfo** info; // Array of PathInfo pointers
    uint32_t length; // Number of entries
} PathEntry;

// Represents components of a split path
typedef struct PathSplit {
    char** parts; // Array of strings for path components
    uint32_t length; // Number of components
} PathSplit;

// Lifecycle management

// Retrieves metadata (caller must free the result)
PathInfo* path_create_info(const char* path);
// Frees a PathInfo object
void path_free_info(PathInfo* info);

// Splits a path into components
PathSplit* path_split(const char* path);
// Frees a PathSplit object
void path_free_split(PathSplit* split);

// Allocates a list of pointers to PathInfo objects
PathEntry* path_create_entry(const char* path);
// Frees a PathEntity structure
void path_free_entry(PathEntry* entity);

// Frees a string returned by path manipulation functions
void path_free_string(char* path);

// Path existence and checks

// Checks if a path exists
bool path_exists(const char* path);
// Checks if a path is a directory
bool path_is_directory(const char* path);
// Checks if a path is a directory
bool path_is_file(const char* path);
// Checks if a path is a symlink
bool path_is_symlink(const char* path);
// Checks if a path has a leading slash (utility)
bool path_has_leading_slash(const char* path);
// Checks if a path has a trailing slash (utility)
bool path_has_trailing_slash(const char* path);

// Path normalization
char* path_add_leading_slash(const char* path); // caller frees the result
char* path_add_trailing_slash(const char* path); // caller frees the result
char* path_remove_leading_slash(const char* path); // caller frees the result
char* path_remove_trailing_slash(const char* path); // caller frees the result

// Path manipulation

// Gets the directory part of a path (caller frees)
char* path_dirname(const char* path);
// Gets the basename of a path (caller frees)
char* path_basename(const char* path);
// Joins two paths (caller frees)
char* path_join(const char* base, const char* sub);

// Traverses a directory tree
PathState path_traverse(const char* base_path, PathEntry* entry, bool recursive);

#endif // ALT_PATH_H
