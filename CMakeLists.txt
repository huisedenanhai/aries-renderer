cmake_minimum_required(VERSION 3.19)
project(aries)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(aries_pre_add_target)
    file(GLOB_RECURSE all_sources CONFIGURE_DEPENDS "*.h" "*.hpp" "*.cpp" "*.c")
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX sources FILES ${all_sources})
endmacro()

macro(aries_post_add_target target)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src)
endmacro()

macro(aries_add_library target)
    aries_pre_add_target()
    add_library(${ARGV})
    aries_post_add_target(${target})
endmacro()

macro(aries_add_executable target)
    aries_pre_add_target()
    add_executable(${ARGV})
    aries_post_add_target(${target})
endmacro()

add_subdirectory(src)
