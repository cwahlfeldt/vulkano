cmake_minimum_required(VERSION 3.20)
project(vulkano 
    VERSION 0.1.0 
    DESCRIPTION "Vulkan renderer library"
    LANGUAGES C
)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Global compiler flags
add_compile_options(
    -Wall 
    -Wextra 
    -Wno-unused-parameter 
    -Wno-enum-conversion 
    -Wno-enum-compare
)

# Optional sanitizer flags
option(USE_SANITIZER "Enable Address Sanitizer" OFF)
if(USE_SANITIZER)
    add_compile_options(-fsanitize=address -g)
    add_link_options(-fsanitize=address)
endif()

# Platform-specific settings
if(WIN32)
    add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
elseif(APPLE)
    add_definitions(-DVK_USE_PLATFORM_METAL_EXT)
elseif(UNIX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(WAYLAND wayland-client)
    if(WAYLAND_FOUND)
        add_definitions(-DVK_USE_PLATFORM_WAYLAND_KHR)
    else()
        add_definitions(-DVK_USE_PLATFORM_XCB_KHR)
    endif()
endif()

# Dependencies
add_subdirectory(lib/SDL EXCLUDE_FROM_ALL)
add_subdirectory(lib/VulkanMemoryAllocator EXCLUDE_FROM_ALL)

# Find Vulkan
find_package(Vulkan REQUIRED)

# Shader compilation
find_program(GLSLC glslc REQUIRED)
file(GLOB SHADER_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.vert
    ${CMAKE_CURRENT_SOURCE_DIR}/shaders/*.frag
)

foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_CURRENT_BINARY_DIR}/shaders
        COMMAND ${GLSLC} ${SHADER} -o ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv
        DEPENDS ${SHADER}
        COMMENT "Compiling shader ${SHADER_NAME}"
    )
    list(APPEND SPV_SHADERS ${CMAKE_CURRENT_BINARY_DIR}/shaders/${SHADER_NAME}.spv)
endforeach()

add_custom_target(shaders ALL DEPENDS ${SPV_SHADERS})

# Library
add_subdirectory(src)

# Examples
add_subdirectory(examples)