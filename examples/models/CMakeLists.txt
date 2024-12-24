# Add ALT related examples
set(EXAMPLES
    perceptron
    xor
    tokenizer
    similarity
    mlp
    mnist # mnist depends upon stb
)

set(INPUT_DIR ${PROJECT_SOURCE_DIR}/examples/models)
set(OUTPUT_DIR ${PROJECT_SOURCE_DIR}/build/examples/models)

foreach(example IN LISTS EXAMPLES)
    add_executable(${example} ${INPUT_DIR}/${example}.c)
    target_link_libraries(${example} alt)
    if (${example} EQUAL mnist)
        target_include_directories(${example} PRIVATE /usr/include/stb /usr/include/uuid ${PROJECT_SOURCE_DIR}/include)
    else()
        target_include_directories(${example} PRIVATE ${PROJECT_SOURCE_DIR}/include)
    endif()
    set_target_properties(${example} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
endforeach()
