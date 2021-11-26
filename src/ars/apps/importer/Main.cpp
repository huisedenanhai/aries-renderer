#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/ResData.h>
#include <ars/runtime/render/res/Mesh.h>
#include <ars/runtime/render/res/Texture.h>
#include <iostream>
#include <stb_image.h>
#include <tiny_gltf.h>

using namespace ars;

std::optional<ResData> import_texture(const std::filesystem::path &path) {
    int width, height, channels;
    auto path_str = path.string();
    // Only loads image as R8G8B8A8_SRGB for simplicity. Other formats may not
    // be supported on some devices.
    unsigned char *pixel_data =
        stbi_load(path_str.c_str(), &width, &height, &channels, 4);
    channels = 4;

    if (!pixel_data) {
        ARS_LOG_CRITICAL("Failed to load image {}", path.string());
        return std::nullopt;
    }

    render::TextureResMeta meta{};
    auto &info = meta.info;
    info.format = render::get_8_bit_texture_format(channels, true);
    info.width = width;
    info.height = height;

    DataSlice pixels{};
    pixels.offset = 0;
    pixels.size = width * height * channels;

    meta.layers.resize(1);
    meta.layers[0].mipmaps.push_back(pixels);

    ResData res{};
    res.ty = render::RES_TYPE_NAME_TEXTURE;
    res.meta = meta;
    res.data.resize(pixels.size);
    std::memcpy(res.data.data(), pixel_data, pixels.size);

    return res;
}

void gltf_warn(const std::filesystem::path &path, const std::string &info) {
    ARS_LOG_WARN("Loading {}: {}", path.string(), info);
}

void gltf_decode_accessor(const tinygltf::Model &gltf,
                          int accessor_index,
                          const unsigned char *&data,
                          size_t &stride,
                          size_t &count) {
    auto &accessor = gltf.accessors[accessor_index];
    auto &buffer_view = gltf.bufferViews[accessor.bufferView];
    auto &buffer = gltf.buffers[buffer_view.buffer];
    stride = accessor.ByteStride(buffer_view);
    auto offset = accessor.byteOffset + buffer_view.byteOffset;
    count = accessor.count;
    data = buffer.data.data() + offset;
}

template <typename T, typename R> R cast_value(const unsigned char *ptr) {
    return static_cast<R>(*reinterpret_cast<const T *>(ptr));
}

void import_gltf_mesh(const std::filesystem::path &path,
                      const tinygltf::Model &gltf) {
    auto target_dir = ".ars" / path.parent_path() / "meshes";
    std::filesystem::create_directories(target_dir);
    for (auto &m : gltf.meshes) {
        for (int prim_index = 0; prim_index < m.primitives.size();
             prim_index++) {
            auto &p = m.primitives[prim_index];
            auto save_path =
                target_dir / fmt::format("{}.{}", m.name, prim_index);

            render::MeshResMeta meta{};
            ResData data{};
            auto &buf = data.data;

            auto add_data = [&](int accessor_index, int elem_size) {
                const unsigned char *ptr = nullptr;
                size_t stride = 0, count = 0;
                gltf_decode_accessor(gltf, accessor_index, ptr, stride, count);
                DataSlice slice{};
                slice.offset = buf.size();
                slice.size = elem_size * count;
                buf.resize(slice.offset + slice.size);

                if (stride == elem_size) {
                    std::memcpy(&buf[slice.offset], ptr, slice.size);
                } else {
                    for (int i = 0; i < count; i++) {
                        std::memcpy(&buf[slice.offset + i * elem_size],
                                    ptr + i * elem_size,
                                    elem_size);
                    }
                }

                return slice;
            };

            // Import attributes
            constexpr int ATTR_COUNT = 4;
            const char *attr_names[ATTR_COUNT] = {
                "POSITION",
                "NORMAL",
                "TANGENT",
                "TEXCOORD_0",
            };

            int attr_types[ATTR_COUNT] = {
                TINYGLTF_TYPE_VEC3,
                TINYGLTF_TYPE_VEC3,
                TINYGLTF_TYPE_VEC4,
                TINYGLTF_TYPE_VEC2,
            };

            int attr_elem_size[ATTR_COUNT] = {
                3 * sizeof(float),
                3 * sizeof(float),
                4 * sizeof(float),
                2 * sizeof(float),
            };

            DataSlice *slices[ATTR_COUNT] = {
                &meta.position,
                &meta.normal,
                &meta.tangent,
                &meta.tex_coord,
            };

            for (int a = 0; a < ATTR_COUNT; a++) {
                auto &attrs = p.attributes;
                auto it = p.attributes.find(attr_names[a]);
                if (it == attrs.end()) {
                    ARS_LOG_WARN("Attribute {} for mesh {}: Not found",
                                 attr_names[a],
                                 save_path.string());
                    continue;
                }

                auto &accessor = gltf.accessors[it->second];
                if (accessor.type != attr_types[a]) {
                    ARS_LOG_WARN("Attribute {} for mesh {}: Type mismatch",
                                 attr_names[a],
                                 save_path.string());
                    continue;
                }

                if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    ARS_LOG_WARN("Attribute {} for mesh {}: Only support float "
                                 "vertex attribute",
                                 attr_names[a],
                                 save_path.string());
                    continue;
                }

                *slices[a] = add_data(it->second, attr_elem_size[a]);
            }

            // Import indices
            std::vector<uint32_t> indices;
            {
                auto accessor_index = p.indices;
                if (accessor_index >= 0) {
                    auto &accessor = gltf.accessors[accessor_index];
                    indices.resize(accessor.count);
                    const unsigned char *ptr = nullptr;
                    size_t stride = 0, count = 0;
                    gltf_decode_accessor(
                        gltf, accessor_index, ptr, stride, count);

                    auto reader = [&](int i) { return ptr + i * stride; };

                    for (int i = 0; i < accessor.count; i++) {
                        switch (accessor.componentType) {
                        case TINYGLTF_COMPONENT_TYPE_BYTE:
                            indices[i] =
                                cast_value<int8_t, uint32_t>(reader(i));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                            indices[i] =
                                cast_value<uint8_t, uint32_t>(reader(i));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_SHORT:
                            indices[i] =
                                cast_value<int16_t, uint32_t>(reader(i));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                            indices[i] =
                                cast_value<uint16_t, uint32_t>(reader(i));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_INT:
                            indices[i] =
                                cast_value<int32_t, uint32_t>(reader(i));
                            break;
                        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                            indices[i] =
                                cast_value<uint32_t, uint32_t>(reader(i));
                            break;
                        default:
                            ARS_LOG_ERROR("Indices of mesh {}: Invalid type.",
                                          save_path.string());
                        }
                    }
                }
            }
            meta.indices.offset = buf.size();
            meta.indices.size = indices.size() * sizeof(uint32_t);
            buf.resize(meta.indices.offset + meta.indices.size);
            std::memcpy(
                &buf[meta.indices.offset], indices.data(), meta.indices.size);

            // Calculate AABB
            auto vert_count = meta.position.size / (3 * sizeof(float));
            auto vertices =
                reinterpret_cast<glm::vec3 *>(&buf[meta.position.offset]);
            meta.aabb =
                math::AABB<float>::from_points(vertices, vertices + vert_count);

            data.ty = render::RES_TYPE_NAME_MESH;
            data.meta = meta;

            data.save(preferred_res_path(save_path.string()));
        }
    }
}

void import_gltf(const std::filesystem::path &path) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model gltf;
    std::string err;
    std::string warn;

    using namespace std::chrono;
    auto start = high_resolution_clock::now();

    auto extension = path.extension();
    auto model_path_str = path.string();

    loader.SetLoadExternalImages(false);

    auto gltf_error = [&](const std::string &info) {
        ARS_LOG_ERROR("Failed to load {}: {}", model_path_str, info);
    };

    bool ret;
    if (extension == ".gltf") {
        ret = loader.LoadASCIIFromFile(&gltf, &err, &warn, model_path_str);
    } else if (extension == ".glb") {
        ret = loader.LoadBinaryFromFile(&gltf, &err, &warn, model_path_str);
    } else {
        std::stringstream ss;
        ss << "Invalid file extension for GLTF: " << extension
           << ", should be .gltf or .glb";
        gltf_error(ss.str());
        return;
    }

    if (!warn.empty()) {
        gltf_warn(path, warn);
    }

    if (!err.empty()) {
        gltf_error(err);
        return;
    }

    if (!ret) {
        ARS_LOG_ERROR("Failed to parse {}", path.string());
        return;
    }

    auto end = high_resolution_clock::now();

    ARS_LOG_INFO("Open gltf file {} takes {}ms",
                 path.string(),
                 duration_cast<milliseconds>(end - start).count());
    import_gltf_mesh(path, gltf);
}

int main() {
    ARS_LOG_INFO("Importing {}", "test.jpg");
    auto res = import_texture("test.jpg");
    if (res.has_value()) {
        res->save(preferred_res_path("test.jpg"));
    }
    auto gltf_path = "FlightHelmetWithLight/FlightHelmetWithLight.gltf";
    ARS_LOG_INFO("Importing {}", gltf_path);
    import_gltf(gltf_path);
}