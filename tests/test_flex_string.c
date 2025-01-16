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
#include "interface/unit_test.h"

// ---------------------- UTF-8 Character test cases ----------------------

typedef struct TestUnitUTF8CharLength {
    int8_t actual;
    int8_t expected;
    const char* input;
} TestUnitUTF8CharLength;

int test_utf8_char_length_logic(TestCase* test) {
    TestUnitUTF8CharLength* unit = (TestUnitUTF8CharLength*) test->unit;
    unit->actual = flex_string_utf8_char_length(*unit->input);

    // Check if the actual length is greater than 0
    ASSERT(
        unit->actual > 0,
        "Invalid UTF-8 leading byte in test case %zu (input: '%s')",
        test->index,
        unit->input
    );

    // Check if the actual length matches the expected length
    ASSERT(
        unit->actual == unit->expected,
        "Invalid UTF-8 byte length in test case %zu (input: '%s', expected: %d, got: %d)",
        test->index,
        unit->input,
        unit->expected,
        unit->actual
    );

    return 0; // Success
}

int test_flex_string_utf8_char_length(void) {
    TestUnitUTF8CharLength units[] = {
        {.input = "a", .expected = 1}, // ASCII 'a'
        {.input = "\x7F", .expected = 1}, // DEL character
        {.input = "\u00A2", .expected = 2}, // Cent sign (¢)
        {.input = "\u20AC", .expected = 3}, // Euro sign (€)
        {.input = "\U0001F600", .expected = 4}, // Grinning Face (😀)
    };

    size_t total_tests = sizeof(units) / sizeof(units[0]);
    TestCase test_cases[total_tests];

    for (size_t i = 0; i < total_tests; i++) {
        test_cases[i].unit = &units[i];
    }

    TestContext context = {
        .test_name = "UTF-8 Character Length",
        .total_tests = total_tests,
        .test_cases = test_cases
    };

    run_unit_tests(&context, test_utf8_char_length_logic, NULL);

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

// ---------------------- Test UTF-8 String Compare ----------------------

typedef struct TestUnitUTF8StringCompare {
    int32_t actual;
    const int32_t expected;
    const char* first;
    const char* second;
} TestUnitUTF8StringCompare;

int test_utf8_string_compare_logic(TestCase* test) {
    TestUnitUTF8StringCompare* unit = (TestUnitUTF8StringCompare*) test->unit;

    unit->actual = flex_string_utf8_string_compare(unit->first, unit->second);

    ASSERT(
        unit->actual == unit->expected,
        "First: %s, Second: %s, Expected: %d, Actual: %d",
        unit->first ? unit->first : "(NULL)",
        unit->second ? unit->second : "(NULL)",
        unit->expected,
        unit->actual
    );

    return 0;
}

int test_flex_string_utf8_string_compare(void) {
    TestUnitUTF8StringCompare units[] = {
        {
            .first = "Hello, world!",
            .second = "Hello, world!",
            .expected = FLEX_STRING_COMPARE_EQUAL,
        },
        {.first = "Hello", .second = "World", .expected = FLEX_STRING_COMPARE_LESS},
        {.first = "World", .second = "Hello", .expected = FLEX_STRING_COMPARE_GREATER},
        {.first = "Hello 🌟", .second = "Hello 🌟", .expected = FLEX_STRING_COMPARE_EQUAL},
        {.first = "Hello 🌟", .second = "Hello", .expected = FLEX_STRING_COMPARE_GREATER},
        {.first = "Hello", .second = "Hello 🌟", .expected = FLEX_STRING_COMPARE_LESS},
        {
            .first = "\xF0\x9F\x98\x80",
            .second = "\xF0\x9F\x98\x81",
            .expected = FLEX_STRING_COMPARE_LESS,
        },
        {
            .first = "\xF0\x9F\x98\x81",
            .second = "\xF0\x9F\x98\x80",
            .expected = FLEX_STRING_COMPARE_GREATER,
        },
        {.first = NULL, .second = "Hello", .expected = FLEX_STRING_COMPARE_INVALID},
        {.first = "Hello", .second = NULL, .expected = FLEX_STRING_COMPARE_INVALID},
    };

    size_t total_tests = sizeof(units) / sizeof(units[0]);
    TestCase test_cases[total_tests];

    for (size_t i = 0; i < total_tests; ++i) {
        test_cases[i].unit = &units[i];
    }

    TestContext context
        = {.test_name = "UTF-8 String Compare", .total_tests = total_tests, .test_cases = test_cases
        };

    return run_unit_tests(&context, test_utf8_string_compare_logic, NULL);
}

// ---------------------- Test UTF-8 String Copy ----------------------

typedef struct TestUnitUTF8StringCopy {
    int32_t actual; // Actual comparison result
    const int32_t expected; // Expected comparison result
    const char* input; // Input string
    const char* copy; // Copy of input string
} TestUnitUTF8StringCopy;

int test_utf8_string_copy_logic(TestCase* test) {
    TestUnitUTF8StringCopy* unit = (TestUnitUTF8StringCopy*) test->unit;

    // Validate the input and copy the string
    unit->copy = flex_string_utf8_string_copy(unit->input);

    // Perform the comparison and store the result
    unit->actual = flex_string_utf8_string_compare(unit->copy, unit->input);

    // Assert the comparison result matches the expected result
    ASSERT(
        unit->actual == unit->expected,
        "Input: %s, Expected: %d, Actual: %d",
        unit->input ? unit->input : "(NULL)",
        unit->expected,
        unit->actual
    );

    return 0; // Success
}

void test_utf8_string_copy_cleanup(TestCase* test) {
    TestUnitUTF8StringCopy* unit = (TestUnitUTF8StringCopy*) test->unit;
    if (unit->copy) {
        free((void*) unit->copy);
        unit->copy = NULL;
    }
}

int test_flex_string_utf8_string_copy(void) {
    TestUnitUTF8StringCopy units[] = {
        {.input = NULL, .expected = FLEX_STRING_COMPARE_INVALID},
        {.input = "", .expected = FLEX_STRING_COMPARE_EQUAL},
        {.input = "Hello, world!", .expected = FLEX_STRING_COMPARE_EQUAL},
        {.input = "안녕하세요, 세상!", .expected = FLEX_STRING_COMPARE_EQUAL},
        {.input = "こんにちは", .expected = FLEX_STRING_COMPARE_EQUAL},
        {.input = "\xF0\x28\x8C\xBC", .expected = FLEX_STRING_COMPARE_INVALID},
    };

    size_t total_tests = sizeof(units) / sizeof(units[0]);
    TestCase test_cases[total_tests];

    for (size_t i = 0; i < total_tests; i++) {
        test_cases[i].unit = &units[i];
    }

    TestContext context = {
        .test_name = "UTF-8 String Copy",
        .total_tests = total_tests,
        .test_cases = test_cases,
    };

    return run_unit_tests(&context, test_utf8_string_copy_logic, test_utf8_string_copy_cleanup);
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
        += run_test_suite("test_flex_string_utf8_char_length", test_flex_string_utf8_char_length);
    result += run_test_suite(
        "test_flex_string_utf8_char_validate", test_flex_string_utf8_char_validate
    );

    // Core UTF-8 String Functions
    result += run_test_suite(
        "test_flex_string_utf8_string_validate", test_flex_string_utf8_string_validate
    );
    result += run_test_suite(
        "test_flex_string_utf8_string_char_length", test_flex_string_utf8_string_char_length
    );
    result += run_test_suite(
        "test_flex_string_utf8_string_byte_length", test_flex_string_utf8_string_byte_length
    );
    result += run_test_suite(
        "test_flex_string_utf8_string_compare", test_flex_string_utf8_string_compare
    );
    result
        += run_test_suite("test_flex_string_utf8_string_copy", test_flex_string_utf8_string_copy);

    // Core FlexString Functions
    result += run_test_suite("test_flex_string_create_and_free", test_flex_string_create_and_free);
    result += run_test_suite(
        "test_flex_string_split_create_and_free", test_flex_string_split_create_and_free
    );

    return result > 0 ? 1 : 0; // Return 1 if any test failed, 0 otherwise
}
