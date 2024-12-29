# Add ALT related examples
set(EXAMPLES
    "q8"
    "fp8"
    "q8row"
    "q4row"
    "types"
)

set(INPUT_DIR ${PROJECT_SOURCE_DIR}/examples/interface/data_types)
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/build/examples/interface/data_types)

foreach(example IN LISTS EXAMPLES)
    add_executable(${example} ${INPUT_DIR}/${example}.c)
    target_link_libraries(${example} alt)
    target_include_directories(${example} PRIVATE ${PROJECT_SOURCE_DIR}/include)
    set_target_properties(${example} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
endforeach()
