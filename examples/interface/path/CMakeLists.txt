# Add ALT related examples
set(EXAMPLES
    "info"
    "list"
    "base"
    "join"
    "entry"
)

set(INPUT_DIR ${PROJECT_SOURCE_DIR}/examples/path)
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/build/examples/path)

foreach(example IN LISTS EXAMPLES)
    add_executable(${example} ${INPUT_DIR}/${example}.c)
    target_link_libraries(${example} alt)
    target_include_directories(${example} PRIVATE ${PROJECT_SOURCE_DIR}/include)
    set_target_properties(${example} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
endforeach()
