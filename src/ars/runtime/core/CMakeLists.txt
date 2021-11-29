add_library(core
        Reflect.h
        Serde.cpp
        Serde.h
        ResData.cpp
        ResData.h
        Res.cpp
        Res.h
        Log.h
        )
aries_setup_common_target(core)

target_link_libraries(core PUBLIC
        glm
        imgui
        RTTR::Core_Lib
        spdlog
        json)

add_subdirectory(math)
add_subdirectory(gui)
add_subdirectory(misc)
add_subdirectory(input)
