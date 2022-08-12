aries_add_executable(aries_importer
        Main.cpp)

target_link_libraries(aries_importer PRIVATE
        engine
        tinygltf
        )
