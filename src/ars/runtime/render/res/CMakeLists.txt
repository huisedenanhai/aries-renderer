aries_target_sources(render PRIVATE
        Material.cpp
        Material.h
        Mesh.cpp
        Mesh.h
        Model.cpp
        Model.h
        Texture.cpp
        Texture.h)

target_link_libraries(render PRIVATE
        tinygltf
        )