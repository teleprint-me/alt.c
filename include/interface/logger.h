/**
 * Copyright © 2024 Austin Berrio
 *
 * @file include/interface/logger.h
 *
 * @brief A simple and lightweight logger written in pure C
 *
 * `logger.c` provides a minimal logging library that allows you to log
 * messages with different severity levels (e.g., DEBUG, INFO, WARN, ERROR) to
 * various outputs like console or file. It's designed to be lightweight and
 * easily integrated into existing C projects.
 */

#ifndef ALT_LOGGER_H
#define ALT_LOGGER_H

#include <errno.h>
#include <pthread.h> // For including mutex functions
#include <stdarg.h> // For variadic function support
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h> // For memory allocation support
#include <string.h> // Include for strerror declaration

/**
 * @brief Enumeration representing different levels of logging.
 *
 * @param LOG_LEVEL_DEBUG Debug level logging.
 * @param LOG_LEVEL_INFO Information level logging.
 * @param LOG_LEVEL_WARN Warning level logging.
 * @param LOG_LEVEL_ERROR  Error level logging.
 */
typedef enum LOG_LEVEL {
    LOG_LEVEL_DEBUG,
    LOG_LEVEL_INFO,
    LOG_LEVEL_WARN,
    LOG_LEVEL_ERROR
} log_level_t;

/**
 * @brief Enumeration representing different types of logging.
 *
 * @param LOG_TYPE_UNKNOWN Unknown log type.
 * @param LOG_TYPE_STREAM Log to a stream (e.g., stdout or stderr).
 * @param LOG_TYPE_FILE Log to a file.
 */
typedef enum LOG_TYPE {
    LOG_TYPE_UNKNOWN,
    LOG_TYPE_STREAM,
    LOG_TYPE_FILE
} log_type_t;

/**
 * @brief Structure representing a logger object.
 *
 * @param log_level The logging level of the logger.
 * @param log_type The type of logger.
 * @param log_type_name The name associated with the logger type.
 * @param file_stream The file stream for writing log messages.
 * @param file_path The path to the log file.
 * @param thread_lock Mutex to ensure thread-safe logging.
 */
typedef struct Logger {
    log_level_t log_level;
    log_type_t log_type;
    const char* log_type_name;
    FILE* file_stream;
    const char* file_path;
    pthread_mutex_t thread_lock;
} logger_t;

/**
 * @brief Sets the logger type and name.
 *
 * This function validates and sets the logger type and name
 * based on the provided logger type.
 *
 * @param logger The logger structure to update.
 * @param log_type The desired logger type.
 *
 * @return True if the type and name were set successfully, false otherwise.
 */
bool set_logger_type_and_name(logger_t* logger, log_type_t log_type);

/**
 * @brief Sets the file path and stream for the logger.
 *
 * This function sets the file path and stream for the logger based on the
 * provided file path. If the file path is NULL, the logger stream is set to
 * stderr, and false is returned to indicate a user error. If the file path
 * is not NULL, an attempt is made to open the file for writing. If the file
 * opening fails, the logger stream is set to stderr, and false is returned
 * to indicate an unexpected condition. If the file path is set successfully,
 * true is returned.
 *
 * @param logger The logger structure to update.
 * @param file_path The path to the log file. Pass NULL to log messages to
 * stderr.
 *
 * @return True if the file path was set successfully, false otherwise.
 */
bool set_logger_file_path_and_stream(logger_t* logger, const char* file_path);

/**
 * @brief Creates a new logger instance.
 *
 * This function dynamically allocates memory for a new logger instance
 * and initializes it with sane default values.
 *
 * @param log_type The desired logger type.
 *
 * @return A pointer to the newly created logger instance, or NULL if memory
 * allocation fails or if the logger type is invalid.
 */
logger_t* logger_new(log_type_t log_type);

/**
 * @brief Creates a new logger instance with the specified log file path and
 * log level.
 *
 * This function creates a new logger instance and initializes it with the
 * specified log level. If a log file path is provided, the logger will attempt
 * to open the file for writing. If the file opening fails, the logger will
 * fall back to writing log messages to stderr.
 *
 * @param log_level The desired log level for the logger.
 * @param log_type The desired log type for the logger.
 * @param file_path The path to the log file. Pass NULL to log messages to
 * stderr.
 *
 * @return A pointer to the newly created logger instance, or NULL if memory
 * allocation fails or if the specified log file cannot be opened.
 */
logger_t* logger_create(log_level_t log_level, log_type_t log_type, const char* file_path);

/**
 * @brief Destroys a logger instance and releases associated resources.
 *
 * This function closes the log file associated with the logger, if any,
 * and frees the memory allocated for the logger instance.
 *
 * @param logger A pointer to the logger instance to be destroyed.
 * @return True if the logger was successfully destroyed, false otherwise.
 */
bool logger_free(logger_t* logger);

/**
 * @brief Logs a message with the specified log level to the logger's file.
 *
 * This function logs a message with the specified log level to the logger's
 * file. If the logger's log level is lower than the specified log level, the
 * message will not be logged.
 *
 * @param logger A pointer to the logger instance to use for logging.
 * @param log_level The log level of the message to be logged.
 * @param format The format string of the message to be logged.
 * @param ... Additional arguments for formatting the message (optional).
 *
 * @return true if the message was successfully logged, false otherwise.
 */
bool logger_message(logger_t* logger, log_level_t log_level, const char* format, ...);

/**
 * @brief Macro for logging messages using a logger instance.
 *
 * This macro provides a convenient shorthand for logging messages using a
 * logger instance. It calls the logger_message function with the specified
 * logger, log level, and message format.
 *
 * @param logger A pointer to the logger instance to use for logging.
 * @param level The log level of the message to be logged.
 * @param format The format string of the message to be logged.
 * @param ... Additional arguments for formatting the message (optional).
 *
 * Example usage:
 * @code{.cpp}
 * LOG(my_logger, LOG_LEVEL_DEBUG, "Debug message: %s\n", "Hello, world!");
 * @endcode
 */
#define LOG(logger, level, format, ...) \
    logger_message((logger), (level), "[%s:%d] " format, __FILE__, __LINE__, ##__VA_ARGS__)

/**
 * @brief Global Logger Object
 *
 * The global logger object provides a centralized logging mechanism for the
 * program. It is statically initialized with default values and can be
 * accessed from any part of the program to log messages at various log levels.
 * The logger ensures thread-safe logging through the use of a mutex.
 *
 * @note The global logger object should not be reinitialized or modified after
 * its creation, as this could lead to undefined behavior. Additionally,
 * explicitly destroying the mutex associated with the logger is not strictly
 * necessary but can be considered good practice, especially in environments
 * where resources are checked meticulously at program exit.
 *
 * @var global_logger
 * The global logger object has the following attributes:
 * - log_level: The logging level of the logger.
 * - log_type: The type of logger.
 * - log_type_name: The name associated with the logger type.
 * - file_stream: The file stream for writing log messages.
 * - file_path: The path to the log file.
 * - thread_lock: Mutex to ensure thread-safe logging.
 *
 * @warning Modifying the global logger object or attempting to reinitialize
 * the mutex after initialization can lead to undefined behavior.
 */
extern logger_t global_logger;

/**
 * @brief Initialize Global Logger
 *
 * Initializes the global logger object with the specified attributes.
 * This function allows customization of the logger's properties such as
 * log level, log type, file stream, and file path. It should be called
 * before using the global logger to ensure proper logging behavior.
 *
 * @param log_level The desired logging level for the logger.
 * @param log_type The type of logger to be used (e.g., stream, file).
 * @param log_type_name The name associated with the logger type.
 * @param file_stream The file stream for writing log messages (NULL if not
 * applicable).
 * @param file_path The path to the log file (NULL if not applicable).
 *
 * @warning Calling this function may alter the behavior of the global logger
 * and should be used with caution. Avoid calling this function after the
 * global logger has been initialized to prevent unintended side effects.
 */
void initialize_global_logger(
    log_level_t log_level,
    log_type_t log_type,
    const char* log_type_name,
    FILE* file_stream,
    const char* file_path
);

/**
 * @brief Convenience macros for logging with the global logger.
 *
 * These macros provide shorthand methods for logging messages at various
 * levels (DEBUG, INFO, WARN, ERROR) using the global logger.
 *
 * @param format The format string for the log message.
 * @param ... Additional arguments for formatting the message (optional).
 *
 * Example usage:
 * @code{.cpp}
 * LOG_DEBUG("Debug message: %s\n", "Hello, world!");
 * LOG_ERROR("Error: %d occurred in function %s\n", error_code, __func__);
 * @endcode
 */
#define LOG_DEBUG(format, ...) LOG(&global_logger, LOG_LEVEL_DEBUG, format, ##__VA_ARGS__)
#define LOG_INFO(format, ...) LOG(&global_logger, LOG_LEVEL_INFO, format, ##__VA_ARGS__)
#define LOG_WARN(format, ...) LOG(&global_logger, LOG_LEVEL_WARN, format, ##__VA_ARGS__)
#define LOG_ERROR(format, ...) LOG(&global_logger, LOG_LEVEL_ERROR, format, ##__VA_ARGS__)

#endif // ALT_LOGGER_H
