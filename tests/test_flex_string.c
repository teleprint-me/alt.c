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

// ---------------------- Test helper functions ----------------------

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

// ---------------------- UTF-8 Character test cases ----------------------

int test_flex_string_utf8_char_length(void) {
    struct TestCase {
        const char* input;
        int expected_length;
    };

    struct TestCase test_cases[] = {
        {"a", 1},          // ASCII 'a'
        {"\x7F", 1},       // DEL character
        {"\u00A2", 1},     // CENT SIGN (¢)
        {"\u20AC", 1},     // EURO SIGN (€)
        {"\U0001F600", 1}, // GRINNING FACE emoji (😀)
    };

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    LOG_INFO("Number of tests: %zu\n", num_tests);

    for (size_t i = 0; i < num_tests; i++) {
        const uint8_t* stream = (const uint8_t*)test_cases[i].input;
        int expected = test_cases[i].expected_length;
        int actual = 0;

        while (*stream) {
            int8_t char_length = flex_string_utf8_char_length(*stream);
            ASSERT(char_length > 0, "Invalid UTF-8 leading byte detected");
            stream += char_length;
            actual++;
        }

        ASSERT(actual == expected, "Incorrect number of UTF-8 characters detected");
    }

    return 0;
}

int test_flex_string_utf8_char_validate(void) {
    struct TestCase {
        const char* input;
        bool expected_valid;
    };

    struct TestCase test_cases[] = {
        {"a", true},             // Valid ASCII
        {"\x7F", true},          // Valid 1-byte UTF-8
        {"\u00A2", true},        // Valid 2-byte UTF-8 (¢)
        {"\u20AC", true},        // Valid 3-byte UTF-8 (€)
        {"\U0001F600", true},    // Valid 4-byte UTF-8 (😀)
        {"\xC0\xAF", false},     // Overlong sequence
        {"\xF0\x28\x8C\xBC", false}, // Invalid 4-byte sequence
    };

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    LOG_INFO("Number of tests: %zu\n", num_tests);

    for (size_t i = 0; i < num_tests; i++) {
        const uint8_t* stream = (const uint8_t*)test_cases[i].input;
        int8_t char_length = flex_string_utf8_char_length(*stream);

        if (char_length == -1) {
            ASSERT(!test_cases[i].expected_valid, "Expected invalid sequence but got valid");
            continue;
        }

        bool is_valid = flex_string_utf8_char_validate(stream, char_length);
        ASSERT(is_valid == test_cases[i].expected_valid, "Incorrect validation result");
    }

    return 0;
}

// ---------------------- FlexString test cases ----------------------

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

    // Core UTF-8 Character Functions
    result += handle_test_case("test_flex_string_utf8_char_length", test_flex_string_utf8_char_length);
    result += handle_test_case("test_flex_string_utf8_char_validate", test_flex_string_utf8_char_validate);

    // Core FlexString Functions
    result += handle_test_case("test_flex_string_create_and_free", test_flex_string_create_and_free);
    result += handle_test_case(
        "test_flex_string_split_create_and_free", test_flex_string_split_create_and_free
    );

    return result > 0 ? 1 : 0; // Return 1 if any test failed, 0 otherwise
}
