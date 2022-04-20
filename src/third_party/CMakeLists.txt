add_subdirectory(glfw)
add_subdirectory(rttr)
add_subdirectory(glm)
add_subdirectory(imgui)
add_subdirectory(lua)
add_subdirectory(vulkan)
add_subdirectory(tinygltf)
add_subdirectory(spdlog)
add_subdirectory(json)
add_subdirectory(mikktspace)

set(SHADERC_SKIP_TESTS ON)
add_subdirectory(shaderc)

set_target_properties(
        glfw rttr_core_lib
        PROPERTIES
        UNITY_BUILD OFF)