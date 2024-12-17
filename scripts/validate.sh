#!/usr/bin/env bash

# ---------------------------------------------------------
# Script: validate.sh
# Purpose:
#   - Validate memory safety using Valgrind.
#   - Ensure ASAN runtime is correctly preloaded for the child process.
#
# Why:
#   - Valgrind and AddressSanitizer (ASAN) can conflict when both intercept memory symbols.
#   - Valgrind complains if ASAN runtime isn’t linked first, so we ensure it is preloaded.
#
# Usage:
#   ./validate.sh <program> [args...]
# ---------------------------------------------------------

# Suppression file for known false positives
SUPPRESSION_FILE="$(pwd)/asan.supp"

# Enable or disable ASAN link order bypass
BYPASS_ASAN_LINK_ORDER=0

# Configure ASAN options
if [[ $BYPASS_ASAN_LINK_ORDER -eq 1 ]]; then
    ASAN_OPTIONS="verify_asan_link_order=0"
else
    ASAN_OPTIONS=""
fi

# Preload AddressSanitizer runtime library
LD_PRELOAD=$(gcc -print-file-name=libasan.so)
# Configure LSAN options (quietly suppress known false positives)
LSAN_OPTIONS="suppressions=${SUPPRESSION_FILE}"

export LD_PRELOAD ASAN_OPTIONS LSAN_OPTIONS

# Check for input arguments
if [[ $# -lt 1 ]]; then
    echo "Usage: $0 <program> [args...]"
    exit 1
fi

# Run the program with Valgrind and ensure LD_PRELOAD propagates
valgrind --leak-check=full \
         --show-leak-kinds=all \
         --trace-children=yes \
         -s "$@"
