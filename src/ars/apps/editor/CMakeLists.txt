add_executable(aries_editor
        Main.cpp)

aries_setup_common_target(aries_editor)
target_link_libraries(aries_editor PRIVATE
        engine
        )
