#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/ResData.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/render/res/Material.h>
#include <ars/runtime/render/res/Mesh.h>
#include <ars/runtime/render/res/Model.h>
#include <ars/runtime/render/res/Texture.h>
#include <fstream>
#include <iostream>
#include <stb_image.h>
#include <tiny_gltf.h>

using namespace ars;

void save(const ResData &data, const std::filesystem::path &path) {
    std::filesystem::create_directories(path.parent_path());
    data.save(preferred_res_path(path));
}

struct TextureImportSettings {
    bool srgb = true;
    render::FilterMode min_filter = render::FilterMode::Linear;
    render::FilterMode mag_filter = render::FilterMode::Linear;
    render::MipmapMode mipmap_mode = render::MipmapMode::Linear;
    render::WrapMode wrap_u = render::WrapMode::Repeat;
    render::WrapMode wrap_v = render::WrapMode::Repeat;
    render::WrapMode wrap_w = render::WrapMode::Repeat;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(TextureImportSettings,
                                   srgb,
                                   min_filter,
                                   mag_filter,
                                   mipmap_mode,
                                   wrap_u,
                                   wrap_v,
                                   wrap_w)
};

template <typename T>
T get_import_setting(const std::filesystem::path &path,
                     const T &default_value) {
    std::filesystem::path setting_path = path.string() + ".import";
    if (!std::filesystem::exists(setting_path)) {
        std::ofstream os(setting_path);
        nlohmann::json js = default_value;
        os << std::setw(2) << js;
        os.close();
        return default_value;
    }
    std::ifstream is(setting_path);
    nlohmann::json js{};
    is >> js;
    is.close();
    return js.get<T>();
}

void import_texture(const std::filesystem::path &path) {
    int width, height, channels;
    auto path_str = path.string();
    // Only loads image as R8G8B8A8_SRGB for simplicity. Other formats may not
    // be supported on some devices.
    unsigned char *pixel_data =
        stbi_load(path_str.c_str(), &width, &height, &channels, 4);
    channels = 4;

    if (!pixel_data) {
        ARS_LOG_ERROR("Failed to load image {}", path.string());
        return;
    }

    auto setting = get_import_setting<TextureImportSettings>(path, {});

    render::TextureResMeta meta{};
    auto &info = meta.info;
    info.format = render::get_8_bit_texture_format(channels, setting.srgb);
    info.width = width;
    info.height = height;
    info.min_filter = setting.min_filter;
    info.mag_filter = setting.mag_filter;
    info.mipmap_mode = setting.mipmap_mode;
    info.wrap_u = setting.wrap_u;
    info.wrap_v = setting.wrap_v;
    info.wrap_w = setting.wrap_w;

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

    save(res, ".ars" / path);
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

void import_gltf_meshes(const std::filesystem::path &path,
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

std::filesystem::path gltf_texture_path(const std::filesystem::path &path,
                                        const tinygltf::Model &gltf,
                                        int index) {
    if (index < 0) {
        return "";
    }
    auto image = gltf.textures[index].source;
    return path.parent_path() / gltf.images[image].uri;
}

void import_gltf_materials(const std::filesystem::path &path,
                           const tinygltf::Model &gltf) {
    auto target_dir = ".ars" / path.parent_path() / "materials";
    std::filesystem::create_directories(target_dir);

    auto tex = [&](int index) -> std::string {
        return gltf_texture_path(path, gltf, index).string();
    };

    for (auto &gltf_mat : gltf.materials) {
        render::MaterialResMeta meta{};
        meta.type = render::MaterialType::MetallicRoughnessPBR;

        auto js = nlohmann::json::object();

        auto &pbr = gltf_mat.pbrMetallicRoughness;
        js["base_color_tex"] = tex(pbr.baseColorTexture.index);
        js["base_color_factor"] = glm::vec4(pbr.baseColorFactor[0],
                                            pbr.baseColorFactor[1],
                                            pbr.baseColorFactor[2],
                                            pbr.baseColorFactor[3]);
        js["metallic_factor"] = (float)pbr.metallicFactor;
        js["roughness_factor"] = (float)pbr.roughnessFactor;
        js["metallic_roughness_tex"] = tex(pbr.metallicRoughnessTexture.index);
        js["normal_tex"] = tex(gltf_mat.normalTexture.index);
        js["normal_scale"] = (float)gltf_mat.normalTexture.scale;
        js["occlusion_tex"] = tex(gltf_mat.occlusionTexture.index);
        js["occlusion_strength"] = (float)gltf_mat.occlusionTexture.strength;
        js["emission_tex"] = tex(gltf_mat.emissiveTexture.index);
        js["emission_factor"] = glm::vec3(gltf_mat.emissiveFactor[0],
                                          gltf_mat.emissiveFactor[1],
                                          gltf_mat.emissiveFactor[2]);

        js["double_sided"] = gltf_mat.doubleSided;

        js["alpha_mode"] = render::MaterialAlphaMode::Opaque;
        if (gltf_mat.alphaMode == "BLEND") {
            js["alpha_mode"] = render::MaterialAlphaMode::Blend;
        }

        ResData data{};
        data.ty = render::RES_TYPE_NAME_MATERIAL;
        nlohmann::json::to_bson(js, data.data);
        meta.properties.offset = 0;
        meta.properties.size = data.data.size();
        data.meta = meta;

        auto save_path = target_dir / gltf_mat.name;
        data.save(preferred_res_path(save_path));
    }
}

void guess_gltf_texture_settings(const std::filesystem::path &path,
                                 const tinygltf::Model &gltf) {
    std::vector<bool> need_srgb{};
    need_srgb.resize(gltf.textures.size());

    for (auto &gltf_mat : gltf.materials) {
        auto base_color = gltf_mat.pbrMetallicRoughness.baseColorTexture.index;
        if (base_color >= 0) {
            need_srgb[base_color] = true;
        }

        auto emission = gltf_mat.emissiveTexture.index;
        if (emission >= 0) {
            need_srgb[emission] = true;
        }
    }

    for (int index = 0; index < gltf.textures.size(); index++) {
        const auto &gltf_tex = gltf.textures[index];
        TextureImportSettings settings{};
        settings.srgb = need_srgb[index];
        if (gltf_tex.sampler >= 0) {
            auto &sampler = gltf.samplers[gltf_tex.sampler];

            settings.min_filter =
                render::gltf_translate_filter_mode(sampler.minFilter);
            settings.mag_filter =
                render::gltf_translate_filter_mode(sampler.magFilter);
            settings.mipmap_mode =
                render::gltf_translate_mipmap_mode(sampler.minFilter);
            settings.wrap_u = render::gltf_translate_wrap_mode(sampler.wrapS);
            settings.wrap_v = render::gltf_translate_wrap_mode(sampler.wrapT);
            // tinygltf mark wrapR as unused, assign a default value here.
            settings.wrap_w = render::WrapMode::Repeat;
        }

        get_import_setting(gltf_texture_path(path, gltf, index), settings);
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
    import_gltf_meshes(path, gltf);
    import_gltf_materials(path, gltf);
    guess_gltf_texture_settings(path, gltf);
}

class Importer : public engine::IApplication {
  public:
    Info get_info() const override {
        Info info{};
        info.name = "Aries Importer";
        info.default_window_logical_width = 128;
        info.default_window_logical_height = 128;
        return info;
    }

    void run() {
        std::vector<std::filesystem::path> gltf_import_tasks{};
        std::vector<std::filesystem::path> tex_import_tasks{};
        using recursive_directory_iterator =
            std::filesystem::recursive_directory_iterator;
        for (const auto &entry :
             recursive_directory_iterator(std::filesystem::current_path())) {
            auto path = std::filesystem::relative(entry.path());
            if (path.root_directory() == ".ars") {
                continue;
            }
            if (entry.is_regular_file()) {
                auto ext = path.extension();
                if (ext == ".jpg" || ext == ".png") {
                    tex_import_tasks.push_back(path);
                }
                if (ext == ".gltf") {
                    gltf_import_tasks.push_back(path);
                }
            }
        }

        // Import gltf first so we can guess texture import settings
        for (auto &task : gltf_import_tasks) {
            ARS_LOG_INFO("Import {}", task.string());
            import_gltf(task);
        }

        for (auto &task : tex_import_tasks) {
            ARS_LOG_INFO("Import {}", task.string());
            import_texture(task);
        }
    }

    void start() override {
        run();
        quit();
    }
};

int main() {
    engine::start_engine(std::make_unique<Importer>());
}