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

#define ASSERT(condition, format, ...) \
    if (!(condition)) { \
        LOG_ERROR("%s: " format "\n", __func__, ##__VA_ARGS__); \
        return 1; \
    }

typedef struct TestContext {
    size_t index; // Index of the current test case
    size_t total_tests; // Total number of test cases
    const char* test_name; // Name of the test
    const void* input; // Input data for the test case
    const void* expected; // Expected output for the test case
    void* output; // Actual output (optional)
    int result; // Test result (0 for success, 1 for failure)
    const char* error_msg; // Error message (optional)
} TestContext;

typedef int (*TestLogic)(TestContext* context); // Test logic
typedef void (*TestCallback)(TestContext* context); // Custom cleanup or logging

int run_tests(TestContext* test_cases, TestLogic test_logic, TestCallback callback) {
    if (!test_cases || !test_logic) {
        LOG_ERROR("%s: Invalid parameters.\n", __func__);
        return -1; // Indicate failure
    }

    LOG_INFO(
        "[RUN] %s: Number of tests: %zu\n", test_cases[0].test_name, test_cases[0].total_tests
    );

    size_t total_tests = test_cases[0].total_tests;
    size_t failures = 0;

    for (size_t i = 0; i < total_tests; i++) {
        TestContext* context = &test_cases[i];
        context->index = i + 1;

        // Execute the test logic
        context->result = test_logic(context);

        // Log failures
        if (context->result != 0) {
            failures++;
            LOG_ERROR(
                "[FAIL] %s: Test case %zu failed: %s\n",
                context->test_name,
                context->index,
                context->error_msg ? context->error_msg : "Unknown error"
            );
        }

        // Optional callback for additional handling
        if (callback) {
            callback(context);
        }
    }

    size_t passed = total_tests - failures;
    LOG_INFO("[RESULT] %s: %zu/%zu tests passed\n", test_cases[0].test_name, passed, total_tests);

    return failures > 0 ? 1 : 0; // Return 1 if any failures, 0 otherwise
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
        int expected_length; // Byte length of the first UTF-8 character
    };

    struct TestCase test_cases[] = {
        {"a", 1}, // ASCII 'a'
        {"\x7F", 1}, // DEL character
        {"\u00A2", 2}, // CENT SIGN (¢)
        {"\u20AC", 3}, // EURO SIGN (€)
        {"\U0001F600", 4}, // GRINNING FACE emoji (😀)
    };

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    LOG_INFO("%s: Number of tests: %zu\n", __func__, num_tests);

    for (size_t i = 0; i < num_tests; i++) {
        const uint8_t* input = (const uint8_t*) test_cases[i].input;
        int expected_length = test_cases[i].expected_length;
        int8_t char_length = flex_string_utf8_char_length(*input);

        ASSERT(
            char_length > 0,
            "Invalid UTF-8 leading byte in test case %zu (input: '%s')",
            i,
            test_cases[i].input
        );
        ASSERT(
            char_length == expected_length,
            "Incorrect UTF-8 byte length in test case %zu (input: '%s', expected: %d, got: %d)",
            i,
            test_cases[i].input,
            expected_length,
            char_length
        );
    }

    return 0;
}

int test_flex_string_utf8_char_validate(void) {
    struct TestCase {
        const char* input;
        bool expected_valid;
    };

    struct TestCase test_cases[] = {
        {"a", true}, // Valid ASCII
        {"\x7F", true}, // Valid 1-byte UTF-8
        {"\u00A2", true}, // Valid 2-byte UTF-8 (¢)
        {"\u20AC", true}, // Valid 3-byte UTF-8 (€)
        {"\U0001F600", true}, // Valid 4-byte UTF-8 (😀)
        {"\xC0\xAF", false}, // Overlong sequence
        {"\xF0\x28\x8C\xBC", false}, // Invalid 4-byte sequence
    };

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    LOG_INFO("%s: Number of tests: %zu\n", __func__, num_tests);

    for (size_t i = 0; i < num_tests; i++) {
        const uint8_t* stream = (const uint8_t*) test_cases[i].input;
        int8_t char_length = flex_string_utf8_char_length(*stream);

        if (char_length == -1) {
            ASSERT(
                !test_cases[i].expected_valid,
                "Expected invalid sequence but got valid: test case %zu (input: '%s')",
                i,
                test_cases[i].input
            );
            continue;
        }

        bool is_valid = flex_string_utf8_char_validate(stream, char_length);
        ASSERT(
            is_valid == test_cases[i].expected_valid,
            "Validation mismatch: test case %zu (input: '%s', expected: %d, got: %d)",
            i,
            test_cases[i].input,
            test_cases[i].expected_valid,
            is_valid
        );
    }

    return 0;
}

/// @todo Add UTF-8 character iterator test cases

// ---------------------- UTF-8 String test cases ----------------------

int test_flex_string_utf8_string_validate(void) {
    struct TestCase {
        const char* input;
        bool expected_valid;
    };

    struct TestCase test_cases[] = {
        {"Hello, world!", true},
        {"Hola, mundo!", true},
        {"こんにちは、世界！", true},
        {"안녕하세요, 세상!", true},
        {"Привет, мир!", true},
        {"你好，世界！", true},
        {"", true},
        {"\xC0\xAF", false}, // Overlong byte sequence
        {"\xF0\x28\x8C\xBC", false}, // Invalid 4-byte sequence
    };

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    LOG_INFO("%s: Number of tests: %zu\n", __func__, num_tests);

    for (size_t i = 0; i < num_tests; ++i) {
        const char* stream = test_cases[i].input;
        bool expected_valid = test_cases[i].expected_valid;
        bool actual_valid = flex_string_utf8_string_validate(stream);

        ASSERT(
            expected_valid == actual_valid,
            "Validation mismatch in test case %zu (input: '%s', expected: %d, got: %d)",
            i,
            stream,
            expected_valid,
            actual_valid
        );
    }
    return 0;
}

int test_flex_string_utf8_string_char_length(void) {
    struct TestCase {
        const char* input;
        size_t expected_length;
    };

    struct TestCase test_cases[] = {
        {"", 0},
        {"Hello!", 6},
        {"Hello, world!", 13},
        {"Jolly ranchers are 25\u00A2!", 22 + 1}, // CENT SIGN (¢ is 2 bytes)
        {"Donuts are only 1\u20AC!", 18 + 1}, // // EURO SIGN (€ is 3 bytes)
        {"Hello 🌟 World!", 13 + 1}, // GLOWING STAR (🌟 is 4 bytes)
        {"Sure thing \U0001F600!", 12 + 1}, // GRINNING FACE (😀 is 4 bytes)
    };

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    LOG_INFO("%s: Number of tests: %zu\n", __func__, num_tests);

    for (size_t i = 0; i < num_tests; ++i) {
        const char* input = test_cases[i].input;
        int32_t expected_length = test_cases[i].expected_length;
        int32_t length = flex_string_utf8_string_char_length(input);
        ASSERT(
            length == expected_length,
            "Validation failed for test case %zu (input: %s, expected: %d, got: %d)",
            i,
            input,
            expected_length,
            length
        );
    }
    return 0;
}

int test_flex_string_utf8_string_byte_length(void) {
    struct TestCase {
        const char* input;
        int32_t expected_length;
    };

    struct TestCase test_cases[] = {
        {"", 0},
        {"Hello!", 6},
        {"Hello, world!", 13},
        {"Jolly ranchers are 25\u00A2!", 22 + 2}, // CENT SIGN (¢ is 2 bytes)
        {"Donuts are only 1\u20AC!", 18 + 3}, // // EURO SIGN (€ is 3 bytes)
        {"Hello 🌟 World!", 13 + 4}, // GLOWING STAR (🌟 is 4 bytes)
        {"Sure thing \U0001F600!", 12 + 4}, // GRINNING FACE (😀 is 4 bytes)
    };

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    LOG_INFO("%s: Number of tests: %zu\n", __func__, num_tests);

    for (size_t i = 0; i < num_tests; ++i) {
        const char* input = test_cases[i].input;
        size_t expected_length = test_cases[i].expected_length;
        size_t actual_length = flex_string_utf8_string_byte_length(input);
        ASSERT(
            actual_length == expected_length,
            "Validation failed for test case %zu (input: %s, expected: %d, got: %d)",
            i,
            input,
            expected_length,
            actual_length
        );
    }

    return 0;
}

int test_flex_string_utf8_string_compare(void) {
    struct TestCase {
        const char* first;
        const char* second;
        int32_t expected_result;
    };

    struct TestCase test_cases[] = {
        {"Hello, world!", "Hello, world!", 0},
        {"Hello", "World", -1},
        {"World", "Hello", 1},
        {"Hello 🌟", "Hello 🌟", 0}, // Equal UTF-8 strings
        {"Hello 🌟", "Hello", 1}, // First string is longer
        {"Hello", "Hello 🌟", -1}, // Second string is longer
        {"\xF0\x9F\x98\x80", "\xF0\x9F\x98\x81", -1}, // 😀 < 😁
        {"\xF0\x9F\x98\x81", "\xF0\x9F\x98\x80", 1}, // 😁 > 😀
        {NULL, "Hello", -2}, // Invalid input (NULL)
        {"Hello", NULL, -2}, // Invalid input (NULL)
    };

    size_t num_tests = sizeof(test_cases) / sizeof(test_cases[0]);
    LOG_INFO("%s: Number of tests: %zu\n", __func__, num_tests);

    for (size_t i = 0; i < num_tests; ++i) {
        const char* first = test_cases[i].first;
        const char* second = test_cases[i].second;
        int32_t expected_result = test_cases[i].expected_result;
        int32_t actual_result = flex_string_utf8_string_compare(first, second);
        ASSERT(
            actual_result == expected_result,
            "Test case %zu failed: expected %d, got %d (first: %s, second: %s)",
            i + 1,
            expected_result,
            actual_result,
            first ? first : "(NULL)",
            second ? second : "(NULL)"
        );
    }

    return 0;
}

int test_copy_logic(TestContext* context) {
    const char* input = (const char*) context->input;
    const char* expected = (const char*) context->expected;

    char* actual = flex_string_utf8_string_copy(input);
    context->output = actual; // Save output for potential inspection

    int result = flex_string_utf8_string_compare(actual, expected);

    if (result != 0) {
        context->error_msg = "String copy did not match expected output.";
        return 1; // Indicate failure
    }
    return 0; // Success
}

void test_copy_cleanup(TestContext* context) {
    if (context->output) {
        free(context->output); // Ensure memory is freed
    }
}

int test_flex_string_utf8_string_copy(void) {
    TestContext test_cases[] = {
        {.input = "", .expected = "", .test_name = "UTF-8 String Copy", .total_tests = 6},
        {.input = "Hello, world!", .expected = "Hello, world!"},
        {.input = "안녕하세요, 세상!", .expected = "안녕하세요, 세상!"},
        {.input = "こんにちは", .expected = "こんにちは"},
        {.input = NULL, .expected = NULL},
        {.input = "\xF0\x28\x8C\xBC", .expected = NULL},
    };

    return run_tests(test_cases, test_copy_logic, test_copy_cleanup);
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
    result
        += handle_test_case("test_flex_string_utf8_char_length", test_flex_string_utf8_char_length);
    result += handle_test_case(
        "test_flex_string_utf8_char_validate", test_flex_string_utf8_char_validate
    );

    // Core UTF-8 String Functions
    result += handle_test_case(
        "test_flex_string_utf8_string_validate", test_flex_string_utf8_string_validate
    );
    result += handle_test_case(
        "test_flex_string_utf8_string_char_length", test_flex_string_utf8_string_char_length
    );
    result += handle_test_case(
        "test_flex_string_utf8_string_byte_length", test_flex_string_utf8_string_byte_length
    );
    result += handle_test_case(
        "test_flex_string_utf8_string_byte_length", test_flex_string_utf8_string_byte_length
    );
    result
        += handle_test_case("test_flex_string_utf8_string_copy", test_flex_string_utf8_string_copy);

    // Core FlexString Functions
    result
        += handle_test_case("test_flex_string_create_and_free", test_flex_string_create_and_free);
    result += handle_test_case(
        "test_flex_string_split_create_and_free", test_flex_string_split_create_and_free
    );

    return result > 0 ? 1 : 0; // Return 1 if any test failed, 0 otherwise
}
