aries_add_executable(playground_lua Lua.cpp)

target_link_libraries(playground_lua PRIVATE liblua)

aries_add_executable(playground_render Render.cpp)

target_link_libraries(playground_render PRIVATE
        engine
        tinygltf
        )

aries_add_executable(playground_hierarchy Hierarchy.cpp)

target_link_libraries(playground_hierarchy PRIVATE engine)

