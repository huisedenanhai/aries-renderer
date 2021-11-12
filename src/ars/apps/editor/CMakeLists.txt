add_executable(aries_editor
        Scene3DView.cpp
        Scene3DView.h
        Main.cpp)

aries_setup_common_target(aries_editor)
target_link_libraries(aries_editor PRIVATE
        engine
        )
