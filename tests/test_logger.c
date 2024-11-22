/*
 * Copyright © 2024 Austin Berrio
 *
 * tests/test_logger.c
 *
 * Build:
 *   gcc -o test_logger source/logger.c tests/test_logger.c -lpthread
 * Run:
 *   ./test_logger
 * Expected output:
 *   [WARN] Global logger warning
 *   [ERROR] Global logger error
 *   [DEBUG] Lazy logger debug
 *   [ERROR] Lazy logger error
 *   [INFO] Should log info
 *   [ERROR] Should log error
 *   Finished all tests!
 * Run: cat test.log
 * Expected output:
 *   [DEBUG] Logging to a file: 1, 2, 3... Done!
 */

#include "logger.h"

#include <stdio.h>

// Test the explicit initialization of the global logger
void test_global_logger_initialization() {
    initialize_global_logger(LOG_LEVEL_WARN, LOG_TYPE_STREAM, "stream", stderr, NULL);
    LOG(&global_logger, LOG_LEVEL_INFO, "This message should not appear\n");
    LOG(&global_logger, LOG_LEVEL_WARN, "Global logger warning\n");
    LOG(&global_logger, LOG_LEVEL_ERROR, "Global logger error\n");
}

// Test lazy initialization and logging behavior
void test_lazy_initialization_and_logging() {
    logger_t* lazy_logger = logger_create(LOG_LEVEL_DEBUG, LOG_TYPE_UNKNOWN, NULL);
    LOG(lazy_logger, LOG_LEVEL_DEBUG, "Lazy logger debug\n");
    LOG(lazy_logger, LOG_LEVEL_ERROR, "Lazy logger error\n");
    logger_free(lazy_logger);
}

// Test logging at different levels
void test_logging_at_different_levels() {
    logger_t* level_logger = logger_create(LOG_LEVEL_INFO, LOG_TYPE_STREAM, NULL);
    LOG(level_logger, LOG_LEVEL_DEBUG, "Should not log debug\n");
    LOG(level_logger, LOG_LEVEL_INFO, "Should log info\n");
    LOG(level_logger, LOG_LEVEL_ERROR, "Should log error\n");
    logger_free(level_logger);
}

// Test logging to a file
void test_logging_to_file() {
    const char* file_path = "test.log";
    logger_t* file_logger = logger_create(LOG_LEVEL_DEBUG, LOG_TYPE_FILE, file_path);
    LOG(file_logger, LOG_LEVEL_DEBUG, "Logging to a file: 1, 2, %d... Done!\n", 3);

    // Clean up
    logger_free(file_logger);
}

int main(void) {
    // Run all test cases
    test_global_logger_initialization();
    test_lazy_initialization_and_logging();
    test_logging_at_different_levels();
    test_logging_to_file();

    puts("Finished all tests!");
    return 0;
}
