aries_target_sources(render PRIVATE
        Gltf.cpp
        Gltf.h
        Texture.cpp
        Texture.h)

target_link_libraries(render PRIVATE
        tinygltf
        )