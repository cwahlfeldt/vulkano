add_executable(spinning_cube spinning_cube.c)

target_link_libraries(spinning_cube PRIVATE vulkano)

# Copy shaders to example binary directory
add_custom_command(
    TARGET spinning_cube POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_BINARY_DIR}/shaders
        $<TARGET_FILE_DIR:spinning_cube>/shaders
)

add_dependencies(spinning_cube shaders)