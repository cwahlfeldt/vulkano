file(GLOB VULKANO_SOURCES *.c)

add_library(vulkano STATIC ${VULKANO_SOURCES})

target_include_directories(vulkano
    PUBLIC 
        ${CMAKE_SOURCE_DIR}/include
    PRIVATE
        ${CMAKE_SOURCE_DIR}/lib/VulkanMemoryAllocator/include
)

target_link_libraries(vulkano
    PUBLIC
        SDL3::SDL3
        Vulkan::Vulkan
)

if(UNIX AND NOT APPLE)
    if(WAYLAND_FOUND)
        target_link_libraries(vulkano PUBLIC ${WAYLAND_LIBRARIES})
    else()
        target_link_libraries(vulkano PUBLIC xcb X11 X11-xcb)
    endif()
endif()

if(APPLE)
    target_link_libraries(vulkano PUBLIC
        "-framework Metal"
        "-framework Foundation"
        "-framework QuartzCore"
    )
endif()