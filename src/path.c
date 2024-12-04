/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/path.h
 *
 * @brief
 */

#include "path.h"
#include "logger.h"

// Helper function to create a PathInfo object using stat
PathInfo* path_create_info(const char* path) {
    if (!path) {
        LOG_ERROR("Path is NULL!\n");
        return NULL;
    }

    struct stat statbuf;
    if (stat(path, &statbuf) != 0) {
        LOG_ERROR("Failed to stat path '%s': %s\n", path, strerror(errno));
        return NULL;
    }

    PathInfo* info = (PathInfo*) malloc(sizeof(PathInfo));
    if (!info) {
        LOG_ERROR("Failed to allocate memory for PathInfo.\n");
        return NULL;
    }

    memset(info, 0, sizeof(PathInfo));

    // Populate PathInfo fields
    info->path = strdup(path);
    info->type = S_ISREG(statbuf.st_mode)    ? FILE_TYPE_REGULAR
                 : S_ISDIR(statbuf.st_mode)  ? FILE_TYPE_DIRECTORY
                 : S_ISLNK(statbuf.st_mode)  ? FILE_TYPE_SYMLINK
                 : S_ISBLK(statbuf.st_mode)  ? FILE_TYPE_BLOCK_DEVICE
                 : S_ISCHR(statbuf.st_mode)  ? FILE_TYPE_CHAR_DEVICE
                 : S_ISFIFO(statbuf.st_mode) ? FILE_TYPE_PIPE
                 : S_ISSOCK(statbuf.st_mode) ? FILE_TYPE_SOCKET
                                             : FILE_TYPE_UNKNOWN;
    info->size = statbuf.st_size;
    info->inode = statbuf.st_ino;
    info->uid = statbuf.st_uid;
    info->gid = statbuf.st_gid;
    info->atime = statbuf.st_atime;
    info->mtime = statbuf.st_mtime;
    info->ctime = statbuf.st_ctime;
    info->permissions = statbuf.st_mode;

    // Set access flags
    info->access = 0;
    if (access(path, R_OK) == 0) {
        info->access |= PATH_ACCESS_READ;
    }
    if (access(path, W_OK) == 0) {
        info->access |= PATH_ACCESS_WRITE;
    }
    if (access(path, X_OK) == 0) {
        info->access |= PATH_ACCESS_EXEC;
    }

    return info;
}

void path_free_info(PathInfo* info) {
    if (!info) {
        return;
    }

    if (info->path) {
        free(info->path);
    }
    if (info->name) {
        free(info->name);
    }
    if (info->parent) {
        free(info->parent);
    }
    free(info);
}

// Helper to print PathInfo
void path_print_info(const PathInfo* info) {
    if (!info) {
        return;
    }

    printf("Path: %s\n", info->path);
    printf("Type: %d\n", info->type);
    printf("Size: %ld bytes\n", info->size);
    printf("Inode: %ld\n", info->inode);
    printf("Owner: UID=%d, GID=%d\n", info->uid, info->gid);
    printf("Access Time: %ld\n", info->atime);
    printf("Modification Time: %ld\n", info->mtime);
    printf("Change Time: %ld\n", info->ctime);
    printf("Permissions: %o\n", info->permissions);

    printf("Access: ");
    if (info->access & PATH_ACCESS_READ) {
        printf("Read ");
    }
    if (info->access & PATH_ACCESS_WRITE) {
        printf("Write ");
    }
    if (info->access & PATH_ACCESS_EXEC) {
        printf("Execute");
    }
    printf("\n");
}

PathState path_exists(const char* path) {
    return access(path, F_OK) == 0 ? PATH_SUCCESS : PATH_ERROR;
}

bool path_has_leading_slash(const char* path) {
    size_t length = strlen(path);
    return (length > 0 && '/' == path[0]) ? true : false;
}

bool path_has_trailing_slash(const char* path) {
    size_t length = strlen(path);
    return (length > 0 && '/' == path[length - 1]) ? true : false;
}

char* path_dir(const char* path) {
    if (!path || *path == '\0') {
        return strdup(".");
    }

    char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        return strdup(".");
    }

    size_t length = last_slash - path;
    char* dir = malloc(length + 1); // Space for '\0'
    if (!dir) {
        return NULL;
    }

    strncpy(dir, path, length);
    dir[length] = '\0';
    return dir;
}

char* path_base(const char* path) {
    if (!path || *path == '\0') {
        return strdup("");
    }

    char* last_slash = strrchr(path, '/');
    if (!last_slash) {
        return strdup(path);
    }

    return strdup(last_slash + 1);
}

char* path_add_leading_slash(const char* path) {
    return strdup(path); // @todo
}

char* path_add_trailing_slash(const char* path) {
    if (!path || *path == '\0') {
        return NULL;
    }

    // If already has a trailing slash, return a duplicate
    if (path_has_trailing_slash(path)) {
        return strdup(path);
    }

    size_t length = strlen(path);
    char* normalized = malloc(length + 2); // Space for '/' and '\0'
    if (!normalized) {
        return NULL;
    }

    strcpy(normalized, path);
    strcat(normalized, "/");
    return normalized;
}

char* path_remove_leading_slash(const char* path) {
    if (!path || *path == '\0') {
        return NULL;
    }

    // If no leading slash, return a duplicate
    if (!path_has_leading_slash(path)) {
        return strdup(path);
    }

    return strdup(path + 1); // Skip the leading slash
}

char* path_remove_trailing_slash(const char* path) {
    if (!path || *path == '\0') {
        return NULL;
    }

    // If no trailing slash, return a duplicate
    if (!path_has_trailing_slash(path)) {
        return strdup(path);
    }

    size_t length = strlen(path) - 1; // Exclude trailing slash
    char* normalized = malloc(length + 1); // Space for '\0'
    if (!normalized) {
        return NULL;
    }

    strncpy(normalized, path, length);
    normalized[length] = '\0';
    return normalized;
}

char* path_join(const char* root_path, const char* sub_path) {
    if (!root_path || !sub_path) {
        return NULL; // Invalid inputs
    }

    // Ensure root_path has a trailing slash
    char* normalized_root = path_add_trailing_slash(root_path);
    if (!normalized_root) {
        return NULL;
    }

    // Ensure sub_path does not have a leading slash
    char* normalized_sub = path_remove_leading_slash(sub_path);
    if (!normalized_sub) {
        free(normalized_root);
        return NULL;
    }

    // Concatenate normalized paths
    size_t new_length = strlen(normalized_root) + strlen(normalized_sub) + 1;
    char* joined_path = malloc(new_length);
    if (!joined_path) {
        free(normalized_root);
        free(normalized_sub);
        return NULL;
    }

    strcpy(joined_path, normalized_root);
    strcat(joined_path, normalized_sub);

    free(normalized_root);
    free(normalized_sub);
    return joined_path;
}

void path_free_string(char* path) {
    if (path) {
        free(path);
    }
}

PathSplit* path_split(const char* path) {
    if (!path || *path == '\0') {
        return NULL;
    }

    PathSplit* split = (PathSplit*) malloc(sizeof(PathSplit));
    if (!split) {
        return NULL;
    }
    split->length = 0;
    split->parts = NULL;

    // Estimate components length and allocate memory
    char* temp = strdup(path);
    char* token = strtok(temp, "/");
    while (token) {
        split->parts = realloc(split->parts, (split->length + 1) * sizeof(char*));
        split->parts[split->length] = strdup(token);
        split->length += 1;
        token = strtok(NULL, "/");
    }

    free(temp);
    return split;
}

void path_free_split(PathSplit* split) {
    if (split) {
        if (split->parts) {
            for (uint32_t i = 0; i < split->length; i++) {
                free(split->parts[i]);
            }
            free(split->parts);
        }
        free(split);
    }
}

// PathEntity* path_create_entity(void) {
//     PathEntity* entity = (PathEntity*) malloc(sizeof(PathEntity));
//     if (!entity) {
//         return NULL;
//     }
//     entity->entries = (struct dirent**) malloc(1 * sizeof(struct dirent*));
//     entity->length = 0;
//     return entity;
// }

// void path_free_entity(PathEntity* entity) {
//     if (entity) {
//         for (uint32_t i = 0; i < entity->length; i++) {
//             free(entity->entries[i]); // Free each dirent
//         }
//         free(entity->entries); // Free the entries array
//         free(entity); // Free the entity itself
//     }
// }

// bool path_traverse(const char* base_path, PathEntity* entity, bool recursive) {
//     if (!entity || !base_path) {
//         return false; // Ensure valid inputs
//     }

//     DIR* dir = opendir(base_path);
//     if (!dir) {
//         LOG_ERROR("Failed to open directory: %s", base_path);
//         return false;
//     }

//     struct dirent* entry;
//     while ((entry = readdir(dir))) {
//         // Skip "." and ".."
//         if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
//             continue;
//         }

//         // Calculate required size for struct dirent (including d_name)
//         size_t entry_size = offsetof(struct dirent, d_name) + strlen(entry->d_name) + 1;

//         // Reallocate entity entries array
//         struct dirent** new_entries = realloc(entity->entries, (entity->length + 1) * sizeof(struct dirent*));
//         if (!new_entries) {
//             LOG_ERROR("Memory allocation failed for dirent entries");
//             closedir(dir);
//             return false;
//         }
//         entity->entries = new_entries;

//         // Allocate memory for the new entry
//         entity->entries[entity->length] = malloc(entry_size);
//         if (!entity->entries[entity->length]) {
//             LOG_ERROR("Memory allocation failed for dirent entry");
//             closedir(dir);
//             return false;
//         }

//         // Copy the entry data
//         memcpy(entity->entries[entity->length], entry, entry_size);
//         entity->length++;

//         // Handle recursion into subdirectories
//         if (recursive && entry->d_type == DT_DIR) {
//             // Construct the full path
//             char* subpath = path_join(base_path, entry->d_name);
//             if (!subpath) {
//                 LOG_ERROR("Failed to construct subpath");
//                 closedir(dir);
//                 return false;
//             }

//             // Recurse into the subdirectory
//             if (!path_traverse(subpath, entity, recursive)) {
//                 free(subpath);
//                 closedir(dir);
//                 return false;
//             }

//             free(subpath);
//         }
//     }

//     closedir(dir);
//     return true;
// }
