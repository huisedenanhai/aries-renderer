add_executable(aries_importer
        Main.cpp)

aries_setup_common_target(aries_importer)
target_link_libraries(aries_importer PRIVATE
        engine
        tinygltf
        )
