set(ARS_FRILL_EXECUTABLE "" CACHE PATH "The frill executable")

if (NOT EXISTS ${ARS_FRILL_EXECUTABLE})
  message(FATAL_ERROR "Please specify the correct path to the frill executable")
endif ()

add_custom_target(shaders ALL
    COMMAND
    ${ARS_FRILL_EXECUTABLE}
    -S="${CMAKE_CURRENT_SOURCE_DIR}"
    -B="${CMAKE_BINARY_DIR}/spv"
    -C="${CMAKE_BINARY_DIR}"

    SOURCES
    frill.json

    BYPRODUCTS
    ${CMAKE_BINARY_DIR}/spv/frill_shaders.hpp

    COMMENT
    "compile shaders to spirv"
    )
