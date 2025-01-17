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

// ---------------------- UTF-8 Character Length ----------------------

typedef struct TestUnitUTF8CharLength {
    int8_t actual;
    const int8_t expected;
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
        .test_name = "UTF-8 Character Length", .total_tests = total_tests, .test_cases = test_cases
    };

    return run_unit_tests(&context, test_utf8_char_length_logic, NULL);
}

// ---------------------- UTF-8 Character Validate ----------------------

typedef struct TestUnitUTF8CharValidate {
    bool actual;
    const bool expected;
    int8_t length;
    const char* input;
} TestUnitUTF8CharValidate;

int test_utf8_char_validate_logic(TestCase* test) {
    // Setup the unit test data
    TestUnitUTF8CharValidate* unit = (TestUnitUTF8CharValidate*) test->unit;

    // Calculate the length of the UTF-8 character sequence
    unit->length = flex_string_utf8_char_length(*unit->input);
    if (unit->length == -1) {
        ASSERT(
            false == unit->expected, // if this is set to false, this is silent
            "Expected invalid sequence but got valid: test case %zu (input: '%0x')",
            test->index,
            *unit->input
        );
    }

    // Validate the UTF-8 character sequence
    unit->actual = flex_string_utf8_char_validate((uint8_t*) unit->input, unit->length);
    ASSERT(
        unit->actual == unit->expected, // this fails when false == unit->expected
        "Test case %zu (input: '%0x') failed: expected %s, got %s",
        test->index,
        *unit->input,
        unit->expected ? "true" : "false",
        unit->actual ? "true" : "false"
    );

    return 0;
}

int test_flex_string_utf8_char_validate(void) {
    // https://www.charset.org/utf-8
    TestUnitUTF8CharValidate units[] = {
        // Valid byte sequences
        {.input = "a", .expected = true}, // Valid ASCII
        {.input = "\x7F", .expected = true}, // Valid 1-byte UTF-8 (DEL)
        {.input = "\u00A2", .expected = true}, // Valid 2-byte UTF-8 (¢)
        {.input = "\u20AC", .expected = true}, // Valid 3-byte UTF-8 (€)
        {.input = "\U0001F600", .expected = true}, // Valid 4-byte UTF-8 (😀)
        // Malicious byte sequences
        {.input = "\xC0\xAF", .expected = false}, // Overlong sequence
        {.input = "\xF0\x28\x8C\xBC", .expected = false}, // Invalid 4-byte sequence
        {.input = "\x80", .expected = false}, // Continuation byte
        {.input = "\xBF", .expected = false}, // Continuation byte
        {.input = "\xC0", .expected = false}, // Invalid start byte
        {.input = "\xC1", .expected = false}, // Invalid start byte
        {.input = "\xE0\x80", .expected = false}, // Overlong encoding
        {.input = "\xF8", .expected = false}, // Invalid 5-byte start byte
        // Deprecated bytes (control characters no longer in use, 128-159)
        {.input = "\xC2\x9F", .expected = true}, // Application Program Command (159)
    };

    size_t total_tests = sizeof(units) / sizeof(units[0]);
    TestCase test_cases[total_tests];

    for (size_t i = 0; i < total_tests; i++) {
        test_cases[i].unit = &units[i];
    }

    TestContext context = {
        .test_name = "UTF-8 Character Validation",
        .total_tests = total_tests,
        .test_cases = test_cases,
    };

    return run_unit_tests(&context, test_utf8_char_validate_logic, NULL);
}

/// @todo Add UTF-8 character iterator test cases

// ---------------------- UTF-8 String Validate ----------------------

typedef struct TestUnitUTF8StringValidate {
    bool actual;
    const bool expected;
    const char* input;
} TestUnitUTF8StringValidate;

int test_utf8_string_validate_logic(TestCase* test) {
    TestUnitUTF8StringValidate* unit = (TestUnitUTF8StringValidate*) test->unit;
    unit->actual = flex_string_utf8_string_validate(unit->input);
    ASSERT(
        unit->actual == unit->expected,
        "Test case %zu (input: '%s') failed: expected %s, got %s",
        test->index,
        unit->input,
        unit->expected ? "true" : "false",
        unit->actual ? "true" : "false"
    );
    return 0;
}

int test_flex_string_utf8_string_validate(void) {
    TestUnitUTF8StringValidate units[] = {
        {.input = "", .expected = true},
        {.input = "Hello, world!", .expected = true},
        {.input = "Hola, mundo!", .expected = true},
        {.input = "こんにちは、世界！", .expected = true},
        {.input = "안녕하세요, 세상!", .expected = true},
        {.input = "Привет, мир!", .expected = true},
        {.input = "你好，世界！", .expected = true},
        {.input = "\xC0\xAF", .expected = false}, // Overlong byte sequence
        {.input = "\xF0\x28\x8C\xBC", .expected = false}, // Invalid 4-byte sequence
    };

    size_t total_tests = sizeof(units) / sizeof(units[0]);
    TestCase test_cases[total_tests];

    for (size_t i = 0; i < total_tests; ++i) {
        test_cases[i].unit = &units[i];
    }

    TestContext context = {
        .test_name = "UTF-8 String Validate Test",
        .total_tests = total_tests,
        .test_cases = test_cases,
    };

    return run_unit_tests(&context, test_utf8_string_validate_logic, NULL);
}

// ---------------------- UTF-8 String Character Length ----------------------

typedef struct TestUnitUTF8StringCharLength {
    int32_t actual;
    const int32_t expected;
    const char* input;
} TestUnitUTF8StringCharLength;

int test_utf8_string_char_length_logic(TestCase* test) {
    TestUnitUTF8StringCharLength* unit = (TestUnitUTF8StringCharLength*) test->unit;
    unit->actual = flex_string_utf8_string_char_length(unit->input);
    ASSERT(
        unit->actual == unit->expected,
        "Validation failed for test case %zu (input: %s, expected: %d, got: %d)",
        test->index,
        unit->input,
        unit->expected,
        unit->actual
    );
    return 0;
}

int test_flex_string_utf8_string_char_length(void) {
    TestUnitUTF8StringCharLength units[] = {
        {.input = "", .expected = 0},
        {.input = "Hello!", .expected = 6},
        {.input = "Hello, world!", .expected = 13},
        {.input = "Jolly ranchers are 25\u00A2!", .expected = 22 + 1}, // CENT SIGN (¢ is 2 bytes)
        {.input = "Donuts are only 1\u20AC!", .expected = 18 + 1}, // // EURO SIGN (€ is 3 bytes)
        {.input = "Hello 🌟 World!", .expected = 13 + 1}, // GLOWING STAR (🌟 is 4 bytes)
        {.input = "Sure thing \U0001F600!", .expected = 12 + 1}, // GRINNING FACE (😀 is 4 bytes)
    };

    size_t total_tests = sizeof(units) / sizeof(units[0]);
    TestCase test_cases[total_tests];

    for (size_t i = 0; i < total_tests; ++i) {
        test_cases[i].unit = &units[i];
    }

    TestContext context
        = {.test_name = "UTF-8 String Char Length Test",
           .total_tests = total_tests,
           .test_cases = test_cases};

    return run_unit_tests(&context, test_utf8_string_char_length_logic, NULL);
}

// ---------------------- UTF-8 String Byte Length ----------------------

typedef struct TestUnitUTF8StringByteLength {
    int32_t actual;
    const int32_t expected;
    const char* input;
} TestUnitUTF8StringByteLength;

int test_utf8_string_byte_length_logic(TestCase* test) {
    TestUnitUTF8StringByteLength* unit = (TestUnitUTF8StringByteLength*) test->unit;
    unit->actual = flex_string_utf8_string_byte_length(unit->input);
    ASSERT(
        unit->actual == unit->expected,
        "Validation failed for test case %zu (input: %s, expected: %d, got: %d)",
        test->index,
        unit->input,
        unit->expected,
        unit->actual
    );
    return 0;
}

int test_flex_string_utf8_string_byte_length(void) {
    TestUnitUTF8StringByteLength units[] = {
        {.input = "", .expected = 0},
        {.input = "Hello!", .expected = 6},
        {.input = "Hello, world!", .expected = 13},
        {.input = "Jolly ranchers are 25\u00A2!", .expected = 22 + 2}, // CENT SIGN (¢ is 2 bytes)
        {.input = "Donuts are only 1\u20AC!", .expected = 18 + 3}, // // EURO SIGN (€ is 3 bytes)
        {.input = "Hello 🌟 World!", .expected = 13 + 4}, // GLOWING STAR (🌟 is 4 bytes)
        {.input = "Sure thing \U0001F600!", .expected = 12 + 4}, // GRINNING FACE (😀 is 4 bytes)
    };

    size_t total_tests = sizeof(units) / sizeof(units[0]);
    TestCase test_cases[total_tests];

    for (size_t i = 0; i < total_tests; ++i) {
        test_cases[i].unit = &units[i];
    }

    TestContext context
        = {.test_name = "UTF-8 String Byte Length Test",
           .total_tests = total_tests,
           .test_cases = test_cases};

    return run_unit_tests(&context, test_utf8_string_byte_length_logic, NULL);
}

// ---------------------- UTF-8 String Compare ----------------------

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
