target_sources(script PUBLIC
        LuaContext.h
        LuaContext.cpp
        )

target_link_libraries(script PRIVATE liblua)

