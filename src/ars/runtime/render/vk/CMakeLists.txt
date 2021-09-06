target_sources(render PRIVATE
        Context.cpp
        Context.h
        Buffer.cpp
        Buffer.h
        Swapchain.cpp
        Swapchain.h
        Material.cpp
        Material.h
        Mesh.cpp
        Mesh.h
        Scene.cpp
        Scene.h
        Texture.cpp
        Texture.h
        Vulkan.cpp
        Vulkan.h
        )

target_link_libraries(render PRIVATE
        vulkan)

add_dependencies(render shaders)

target_include_directories(render PRIVATE
        ${CMAKE_BINARY_DIR}/spv)