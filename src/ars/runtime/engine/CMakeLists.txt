add_library(engine
        ResRef.cpp
        ResRef.h
        Spawn.cpp
        Spawn.h
        Entity.cpp
        Entity.h
        Entity.Editor.cpp
        Entity.Editor.h
        Engine.cpp
        Engine.h)
aries_setup_common_target(engine)

target_link_libraries(engine PUBLIC
        core
        render)

add_subdirectory(components)