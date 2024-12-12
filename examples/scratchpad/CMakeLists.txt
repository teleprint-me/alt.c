# Add ALT related examples
set(EXAMPLES
    "array"
    "compare_float"
    "random"
    "char"
)

set(INPUT_DIR ${PROJECT_SOURCE_DIR}/examples/scratchpad)
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/build/examples/scratchpad)

foreach(example IN LISTS EXAMPLES)
    add_executable(${example} ${INPUT_DIR}/${example}.c)
    target_link_libraries(${example} alt)
    target_include_directories(${example} PRIVATE ${PROJECT_SOURCE_DIR}/include)
    set_target_properties(${example} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
endforeach()
