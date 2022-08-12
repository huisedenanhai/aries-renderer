aries_add_library(engine
        Spawn.cpp
        Spawn.h
        Entity.cpp
        Entity.h
        Entity.Editor.cpp
        Entity.Editor.h
        Engine.cpp
        Engine.h)

target_link_libraries(engine PUBLIC
        core
        render)

add_subdirectory(components)
add_subdirectory(gui)
