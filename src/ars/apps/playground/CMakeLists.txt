add_executable(playground_lua Lua.cpp)
aries_setup_common_target(playground_lua)

target_link_libraries(playground_lua PRIVATE liblua)

add_executable(playground_render Render.cpp)
aries_setup_common_target(playground_render)

target_link_libraries(playground_render PRIVATE
        engine
        tinygltf
        )

add_executable(playground_hierarchy Hierarchy.cpp)
aries_setup_common_target(playground_hierarchy)

target_link_libraries(playground_hierarchy PRIVATE engine)

