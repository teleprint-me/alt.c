/**
 * Copyright © 2024 Austin Berrio
 *
 * @file src/interface/unit_test.c
 */

#include "interface/unit_test.h"

int run_unit_tests(TestContext* context, TestLogic logic, TestCallback callback) {
    if (!context || !context->test_cases || !logic) {
        LOG_ERROR("%s: Invalid parameters.\n", __func__);
        return -1;
    }

    LOG_INFO("[RUN] %s: Number of tests: %zu\n", context->test_name, context->total_tests);

    size_t failures = 0;

    for (size_t i = 0; i < context->total_tests; i++) {
        TestCase* test_case = &context->test_cases[i];
        test_case->index = i + 1;

        int result = logic(test_case);

        if (result != 0) {
            failures++;
            LOG_ERROR("[FAIL] %s: Test case %zu failed.\n", context->test_name, test_case->index);
        }

        if (callback) {
            callback(test_case);
        }
    }

    size_t passed = context->total_tests - failures;
    LOG_INFO(
        "[RESULT] %s: %zu/%zu tests passed\n", context->test_name, passed, context->total_tests
    );

    return failures > 0 ? 1 : 0;
}

int run_test_suite(const char* test_name, int (*test_func)(void)) {
    LOG_INFO("[RUN] %s\n", test_name);
    int result = test_func();
    if (result == 0) {
        LOG_INFO("[PASS] %s\n", test_name);
    } else {
        LOG_ERROR("[FAIL] %s\n", test_name);
    }
    return result;
}
