add_library(core
        Log.cpp
        Log.h
        )
aries_setup_common_target(core)

target_link_libraries(core PUBLIC
        glm
        imgui
        RTTR::Core_Lib)

add_subdirectory(math)
add_subdirectory(gui)
add_subdirectory(misc)
add_subdirectory(scene)
add_subdirectory(input)
