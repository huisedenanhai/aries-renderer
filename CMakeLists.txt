cmake_minimum_required(VERSION 3.15)
project(aries)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

macro(aries_setup_common_target target)
    target_include_directories(${target} PRIVATE ${CMAKE_SOURCE_DIR}/src)
endmacro()

macro(aries_target_sources)
    target_sources(${ARGV})
endmacro()

add_subdirectory(src)

