aries_target_sources(render PRIVATE
        Model.cpp
        Model.h
        Texture.cpp
        Texture.h)

target_link_libraries(render PRIVATE
        tinygltf
        )