# Add ALT tests

# Define test executables and input directories
set(C_TESTS
    "test_logger"
    "test_flex_string"
    "test_flex_array"
    "test_activation"
)

# Set input and output directories
set(INPUT_DIR ${PROJECT_SOURCE_DIR}/tests)
set(OUTPUT_DIR ${CMAKE_BINARY_DIR}/tests)

# Log test directory and output directory
message(STATUS "Test Sources Directory: ${INPUT_DIR}")
message(STATUS "Test Executables Directory: ${OUTPUT_DIR}")
message(STATUS "Test Names: ${C_TESTS}")

# Create test executables
foreach (test IN LISTS C_TESTS)
    add_executable(${test} ${INPUT_DIR}/${test}.c)
    target_link_libraries(${test} PUBLIC "alt")
    target_include_directories(${test} PRIVATE ${PROJECT_SOURCE_DIR}/include)
    set_target_properties(${test} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
    add_custom_target("run_${test}" COMMAND ${test} DEPENDS ${test} COMMENT "Running tests for ${test}")
    add_test(NAME ${test} COMMAND ${test} WORKING_DIRECTORY ${OUTPUT_DIR})
endforeach()
