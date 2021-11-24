add_subdirectory(glfw)
add_subdirectory(rttr)
add_subdirectory(glm)
add_subdirectory(imgui)
add_subdirectory(lua)
add_subdirectory(vulkan)
add_subdirectory(tinygltf)
add_subdirectory(spdlog)
add_subdirectory(json)

set_target_properties(
        glfw rttr_core_lib
        PROPERTIES
        UNITY_BUILD OFF)