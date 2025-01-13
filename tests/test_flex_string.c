/**
 * @file tests/test_flex_string.c
 * @brief Tests for the flex_string library.
 * @note All functions must return 0 on success, and non-zero on failure.
 */

// Standard C libraries
#include <stdio.h>
#include <string.h>

// ALT libraries
#include "interface/flex_string.h"
#include "interface/logger.h"

#define ASSERT(condition, message) \
    if (!(condition)) { \
        LOG_ERROR("%s: %s\n", __func__, message); \
        return 1; \
    }

int handle_test_case(const char* test_name, int (*test_func)(void)) {
    LOG_INFO("[RUN] %s\n", test_name);
    int result = test_func();
    if (result == 0) {
        LOG_INFO("[PASS] %s\n", test_name);
    } else {
        LOG_ERROR("[FAIL] %s\n", test_name);
    }
    return result;
}

// Test flex_string_create and flex_string_free.
int test_flex_string_create_and_free(void) {
    char* string = "Hello, world!";
    FlexString* fixture = flex_string_create(string);

    ASSERT(fixture != NULL, "flex_string_create returned NULL");
    ASSERT(strlen(string) == strlen(fixture->data), "String lengths do not match");
    ASSERT(strcmp(string, fixture->data) == 0, "String data does not match");

    flex_string_free(fixture);
    return 0;
}

int test_flex_string_split_create_and_free(void) {
    uint32_t initial_capacity = 10;
    FlexStringSplit* fixture = flex_string_create_split(initial_capacity);

    ASSERT(fixture != NULL, "flex_string_create_split returned NULL");
    ASSERT(fixture->length == 0, "Initial length is not zero");
    ASSERT(fixture->capacity == initial_capacity, "Initial capacity is incorrect");

    flex_string_free_split(fixture);
    return 0;
}

int main(void) {
    global_logger.log_level = LOG_LEVEL_DEBUG;

    int result = 0;
    result += handle_test_case("test_flex_string_create_and_free", test_flex_string_create_and_free);
    result += handle_test_case("test_flex_string_split_create_and_free", test_flex_string_split_create_and_free);

    return result > 0 ? 1 : 0; // Return 1 if any test failed, 0 otherwise
}
