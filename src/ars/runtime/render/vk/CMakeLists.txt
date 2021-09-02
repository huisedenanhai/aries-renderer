target_sources(render PRIVATE
    VulkanContext.cpp
    VulkanContext.h
    VulkanBuffer.cpp
    VulkanBuffer.h
    VulkanSwapchain.cpp
    VulkanSwapchain.h
    VulkanMaterial.cpp
    VulkanMaterial.h
    VulkanMesh.cpp
    VulkanMesh.h
    VulkanScene.cpp
    VulkanScene.h
    VulkanTexture.cpp
    VulkanTexture.h
    Vulkan.cpp
    Vulkan.h
    )

target_link_libraries(render PRIVATE
    vulkan)

add_dependencies(render shaders)

target_include_directories(render PRIVATE
    ${CMAKE_BINARY_DIR}/spv)