aries_add_executable(aries_editor
        Entity.cpp
        Entity.h
        FileBrowser.cpp
        FileBrowser.h
        SaveAsModal.cpp
        SaveAsModal.h
        Scene3DView.cpp
        Scene3DView.h
        Main.cpp)

target_link_libraries(aries_editor PRIVATE
        engine
        )
