/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/path.h
 *
 * @brief
 */

#include "path.h"
#include "logger.h"

bool path_exists(const char* path) {
    return access(path, F_OK) == 0 ? true : false;
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

PathEntity* path_create_entity(void) {
    PathEntity* entity = (PathEntity*) malloc(sizeof(PathEntity));
    if (!entity) {
        return NULL;
    }
    entity->entries = (struct dirent**) malloc(1 * sizeof(struct dirent*));
    entity->length = 0;
    return entity;
}

void path_free_entity(PathEntity* entity) {
    if (entity) {
        for (uint32_t i = 0; i < entity->length; i++) {
            free(entity->entries[i]); // Free each dirent
        }
        free(entity->entries); // Free the entries array
        free(entity); // Free the entity itself
    }
}

bool path_traverse(const char* base_path, PathEntity* entity, bool recursive) {
    if (!entity || !base_path) {
        return false; // Ensure valid inputs
    }

    DIR* dir = opendir(base_path);
    if (!dir) {
        LOG_ERROR("Failed to open directory: %s", base_path);
        return false;
    }

    struct dirent* entry;
    while ((entry = readdir(dir))) {
        // Skip "." and ".."
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        // Calculate required size for struct dirent (including d_name)
        size_t entry_size = offsetof(struct dirent, d_name) + strlen(entry->d_name) + 1;

        // Reallocate entity entries array
        struct dirent** new_entries = realloc(entity->entries, (entity->length + 1) * sizeof(struct dirent*));
        if (!new_entries) {
            LOG_ERROR("Memory allocation failed for dirent entries");
            closedir(dir);
            return false;
        }
        entity->entries = new_entries;

        // Allocate memory for the new entry
        entity->entries[entity->length] = malloc(entry_size);
        if (!entity->entries[entity->length]) {
            LOG_ERROR("Memory allocation failed for dirent entry");
            closedir(dir);
            return false;
        }

        // Copy the entry data
        memcpy(entity->entries[entity->length], entry, entry_size);
        entity->length++;

        // Handle recursion into subdirectories
        if (recursive && entry->d_type == DT_DIR) {
            // Construct the full path
            char* subpath = path_join(base_path, entry->d_name);
            if (!subpath) {
                LOG_ERROR("Failed to construct subpath");
                closedir(dir);
                return false;
            }

            // Recurse into the subdirectory
            if (!path_traverse(subpath, entity, recursive)) {
                free(subpath);
                closedir(dir);
                return false;
            }

            free(subpath);
        }
    }

    closedir(dir);
    return true;
}
