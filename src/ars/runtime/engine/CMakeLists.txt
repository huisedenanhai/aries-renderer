add_library(engine
        Engine.cpp
        Engine.h)
aries_setup_common_target(engine)

target_link_libraries(engine PUBLIC
        core
        render
        )
