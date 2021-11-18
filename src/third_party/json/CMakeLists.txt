file(GLOB NLOHMANN_JSON_FILES
        include/*)
add_library(json INTERFACE ${NLOHMANN_JSON_FILES})
target_include_directories(json INTERFACE include)