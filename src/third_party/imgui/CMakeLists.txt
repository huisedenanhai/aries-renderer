file(GLOB IMGUIZMO_FILES
        include/imguizmo/*.h
        include/imguizmo/*.cpp
        )
add_library(imgui
        include/imgui/imconfig.h
        include/imgui/imgui_demo.cpp
        include/imgui/imgui_draw.cpp
        include/imgui/imgui_internal.h
        include/imgui/imgui_tables.cpp
        include/imgui/imgui_widgets.cpp
        include/imgui/imgui.cpp
        include/imgui/imgui.h
        include/imgui/imstb_rectpack.h
        include/imgui/imstb_textedit.h
        include/imgui/imstb_truetype.h
        ${IMGUIZMO_FILES}
        )

target_include_directories(imgui PUBLIC include)

add_library(imgui_glfw
        include/imgui/imgui_impl_glfw.h
        include/imgui/imgui_impl_glfw.cpp
        )

target_link_libraries(imgui_glfw PRIVATE glfw)