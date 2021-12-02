add_library(physics
        World.cpp
        World.h)

aries_setup_common_target(physics)
target_link_libraries(physics PUBLIC core)
