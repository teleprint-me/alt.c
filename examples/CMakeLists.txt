# Add ALT related examples
set(EXAMPLES 
    random
    tensors
)

# Create executables for simple ALT examples
# Loop over each example and create an executable
foreach(example IN LISTS EXAMPLES)
    add_executable(${example} ${PROJECT_SOURCE_DIR}/examples/${example}.c)
    target_link_libraries(${example} alt)
    target_include_directories(${example} PRIVATE ${PROJECT_SOURCE_DIR}/include)
    set_target_properties(${example} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/build/examples)
endforeach()
