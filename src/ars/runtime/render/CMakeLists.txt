add_library(render
        ISwapchain.cpp
        ISwapchain.h
        IRenderContext.cpp
        IRenderContext.h
        IBuffer.cpp
        IBuffer.h
        ITexture.cpp
        ITexture.h
        IScene.cpp
        IScene.h
        IMesh.cpp
        IMesh.h
        IMaterial.cpp
        IMaterial.h)

target_link_libraries(render PUBLIC glfw)
aries_setup_common_target(render)

add_subdirectory(vk)
