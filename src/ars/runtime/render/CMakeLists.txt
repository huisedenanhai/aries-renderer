add_library(render
        Common.h
        IWindow.cpp
        IWindow.h
        IContext.cpp
        IContext.h
        ITexture.cpp
        ITexture.h
        IScene.cpp
        IScene.h
        IMesh.cpp
        IMesh.h
        IMaterial.cpp
        IMaterial.h)

target_link_libraries(render PUBLIC
        glm
        imgui
        core)
target_link_libraries(render PRIVATE glfw imgui_glfw)
aries_setup_common_target(render)

add_subdirectory(vk)
add_subdirectory(res)
