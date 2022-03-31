add_library(render
        Common.cpp
        Common.h
        IEffect.cpp
        IEffect.h
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

target_link_libraries(render PUBLIC core imgui)
target_link_libraries(render PRIVATE glfw imgui_glfw shaderc)
aries_setup_common_target(render)

add_subdirectory(vk)
add_subdirectory(res)
