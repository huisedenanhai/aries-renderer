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

target_link_libraries(render PUBLIC glm)
target_link_libraries(render PRIVATE glfw)
aries_setup_common_target(render)

add_subdirectory(vk)
