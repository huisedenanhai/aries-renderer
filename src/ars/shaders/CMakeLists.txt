set(ARS_FRILL_EXECUTABLE "" CACHE PATH "The frill executable")

if (NOT EXISTS ${ARS_FRILL_EXECUTABLE})
    message(FATAL_ERROR "Please specify the correct path to ARS_FRILL_EXECUTABLE")
endif ()

file(GLOB_RECURSE all_shader_sources CONFIGURE_DEPENDS "*")
aries_pre_add_target()
add_custom_target(shaders ALL
        COMMAND
        ${ARS_FRILL_EXECUTABLE}
        -s="${CMAKE_CURRENT_SOURCE_DIR}"
        -o="${CMAKE_BINARY_DIR}/spv"

        SOURCES ${all_shader_sources}

        BYPRODUCTS
        ${CMAKE_BINARY_DIR}/spv/frill.h
        ${CMAKE_BINARY_DIR}/spv/frill.cpp

        COMMENT
        "compile shaders to spirv"
        )

aries_add_library(frill_shaders
        ${CMAKE_BINARY_DIR}/spv/frill.h
        ${CMAKE_BINARY_DIR}/spv/frill.cpp
        )