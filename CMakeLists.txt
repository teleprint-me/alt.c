# alt.c CMakeLists.txt

cmake_minimum_required(VERSION 3.14)
project("alt" VERSION 0.1.0 DESCRIPTION "ANSI C library for inferencing, training, and finetuning Mistral.")

# Set C and C++ standard rules
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Add sanitizers for memory safety
# Ref: https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
# Ref: https://developers.redhat.com/blog/2018/03/21/compiler-and-linker-flags-gcc

# Common warning flags
set(COMMON_WARNING_FLAGS "-Wall -Wextra -Wpedantic -Werror -Wformat-security -Wshadow -fexceptions")

# Additional Debug-only flags (sanitizers and memory safety checks)
set(DEBUG_SANITIZERS "-fsanitize=address,undefined -fno-omit-frame-pointer")
set(DEBUG_EXTRA_WARNINGS "-Wformat -Wnull-dereference -Wdouble-promotion")

# Static analysis flags for catching common security issues
set(DEBUG_ANALYSIS "-Wanalyzer-double-free -Wanalyzer-file-leak -Wanalyzer-malloc-leak -Wanalyzer-null-dereference -Wanalyzer-out-of-bounds -Wanalyzer-va-list-leak")

# Enable debugging level 3 for macros
set(DEBUG_FLAGS "-g3")

# Apply flags conditionally based on build type
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_WARNING_FLAGS} ${DEBUG_SANITIZERS} ${DEBUG_EXTRA_WARNINGS} ${DEBUG_ANALYSIS} ${DEBUG_FLAGS}")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_WARNING_FLAGS}")
endif()

# Enable Shared Libraries option
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

# Set output directories for binaries and libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Define the core C library (still feeling this out)
set(
    # sources are still in early development
    C_SOURCES
    # Unit Testing
    "src/interface/unit_test.c"
    # Algorithms
    "src/algorithm/hash_table.c"
    "src/algorithm/binary_tree.c"
    # Interfaces
    "src/interface/logger.c"
    "src/interface/path.c"
    "src/interface/data_types.c"
    "src/interface/random.c"
    "src/interface/activation.c"
    "src/interface/flex_array.c"
    "src/interface/flex_string.c"
    "src/tensors.c" # work in progress
    # Vulkan backend
    "src/vk/instance.c"
    "src/vk/device.c"
    # "src/vk/shader.c"
    # Models
    "src/model/magic.c"
    "src/model/tokenizer.c"
    "src/model/mistral.c"
)
add_library("alt" ${C_SOURCES})

# Include headers
target_include_directories("alt" PUBLIC include)

# Enable testing
enable_testing()
add_subdirectory(tests)

# Add examples
# Define subdirectories
set(EXAMPLES 
    "examples"
    "examples/scratchpad"
    "examples/interface"
    "examples/interface/path"
    "examples/interface/data_types"
    "examples/algorithm"
    "examples/vk"
    "examples/models"
)
foreach (example IN LISTS EXAMPLES)
    add_subdirectory(${example})
endforeach()

# Link necessary libraries (e.g., math, real-time library, Vulkan if Linux)
if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    target_compile_options("alt" PRIVATE -pthread)
    target_link_libraries("alt" PUBLIC m pcre2-8 uuid utf8proc rt vulkan)
endif()

# Custom clean target for removing generated files (optional)
add_custom_target(clean_all
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/bin ${CMAKE_BINARY_DIR}/lib
    COMMENT "Cleaning all generated files"
)
