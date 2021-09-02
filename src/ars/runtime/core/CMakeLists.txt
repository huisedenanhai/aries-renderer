add_library(core)
aries_setup_common_target(core)

target_link_libraries(core PUBLIC
        glm)

add_subdirectory(math)
add_subdirectory(misc)
add_subdirectory(scene)
