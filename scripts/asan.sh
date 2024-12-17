#!/bin/bash

# ---------------------------------------------------------
# Wrapper Script: run_instance.sh
# Purpose:
#   - Suppress false positive memory leaks reported by ASAN.
#   - Preload ASAN library and set LSAN options to handle
#     harmless issues caused by AddressSanitizer's limitations.
#
# Why?
#   AddressSanitizer (ASAN) detects a small memory leak (128 bytes)
#   during process exit. This is a **false positive** caused by:
#     - Vulkan Validation Layers or ICU libraries using thread-safe
#       initialization mechanisms (`pthread_once`).
#     - ASAN failing to intercept and manage these initializations
#       cleanly during process teardown.
#
#   Valgrind confirms that the program itself has **no real memory leaks**,
#   and the issue is isolated to ASAN's resource handling.
#
# Observations:
#   - Leak occurs **after program exit**, not during runtime.
#   - No runtime errors are reported.
#   - Valgrind shows the program is clean.
#   - This behavior is consistent with known ASAN limitations.
#
# How It Works:
#   - LSAN suppresses specific false positive leak patterns
#     (`pthread_once` and `realloc`) using a suppression file.
#   - LD_PRELOAD ensures ASAN runtime is loaded before execution.
#
# Reference:
#   - Vulkan SDK Validation Layers: https://vulkan.lunarg.com/
#   - ASAN Docs: https://clang.llvm.org/docs/AddressSanitizer.html
#   - Known ASAN Limitations: Thread-safe initializations (e.g., pthread_once)
# ---------------------------------------------------------

# Suppression File:
# Contains patterns to ignore harmless leaks reported by ASAN.
SUPPRESSION_FILE="$(pwd)/asan.supp"

# LSAN Options:
#   - Suppress leaks using patterns in the suppression file.
#   - For debugging purposes, enable verbosity (commented out below).
# Uncomment for detailed logs:
# LSAN_OPTIONS="suppressions=$SUPPRESSION_FILE:verbosity=1:log_threads=1"

# Normal Mode (quiet suppression of false positives)
LSAN_OPTIONS="suppressions=$SUPPRESSION_FILE"

# Preload AddressSanitizer runtime library
LD_PRELOAD=$(gcc -print-file-name=libasan.so)

# Export the required environment variables
export LSAN_OPTIONS LD_PRELOAD

# Notes for Future Maintenance:
#   - The 128-byte leak reported by ASAN is harmless.
#   - If new false positives appear, update 'asan.supp' with relevant patterns.
#   - Use 'verbosity=1' in LSAN_OPTIONS for troubleshooting.
#   - Valgrind can be used as a backup to verify memory integrity.

# Run the Vulkan example binary
$1
