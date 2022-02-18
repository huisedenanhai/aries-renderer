add_library(core
        Core.cpp
        Core.h
        Profiler.cpp
        Profiler.h
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
        imgui
        glm
        RTTR::Core_Lib
        spdlog
        json)

add_subdirectory(math)
add_subdirectory(misc)
add_subdirectory(input)
