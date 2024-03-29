target_sources(render PRIVATE
        Bindless.cpp
        Bindless.h
        ShaderLib.cpp
        Profiler.cpp
        Profiler.h
        Effect.cpp
        Effect.h
        Sky.cpp
        Sky.h
        Lut.cpp
        Lut.h
        RenderGraph.cpp
        RenderGraph.h
        RenderPass.cpp
        RenderPass.h
        View.cpp
        View.h
        RenderTarget.cpp
        RenderTarget.h
        Descriptor.cpp
        Descriptor.h
        ImGui.cpp
        ImGui.h
        Pipeline.cpp
        Pipeline.h
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
        vulkan
        frill_shaders)

add_dependencies(render shaders)

target_include_directories(render PRIVATE
        ${CMAKE_BINARY_DIR}/spv)

add_subdirectory(features)
add_subdirectory(materials)
