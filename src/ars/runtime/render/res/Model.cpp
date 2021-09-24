#include "Model.h"
#include "../IContext.h"
#include "../IMesh.h"
#include "../ITexture.h"
#include "Texture.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/misc/Visitor.h>
#include <chrono>
#include <sstream>
#include <tiny_gltf.h>

namespace ars::render {
Model::Camera::Type Model::Camera::type() const {
    if (std::holds_alternative<PerspectiveData>(data)) {
        return Model::Camera::Perspective;
    }
    return Model::Camera::Orthographic;
}

glm::mat4 Model::Camera::projection_matrix(float aspect) const {
    return std::visit(make_visitor(
                          [](const PerspectiveData &pers) {
                              // TODO
                              return glm::identity<glm::mat4>();
                          },
                          [](const OrthographicData &ortho) {
                              // TODO
                              return glm::identity<glm::mat4>();
                          }),
                      data);
}

namespace {
void gltf_warn(const std::filesystem::path &path, const std::string &info) {
    std::stringstream ss;
    ss << "Loading " << path << ": " << info;
    log_warn(ss.str());
}

template <typename T, typename R> R cast_value(const unsigned char *ptr) {
    return static_cast<R>(*reinterpret_cast<const T *>(ptr));
}

template <typename T, typename Reader>
std::vector<T> read_to_vec(Reader &&reader, size_t n) {
    std::vector<T> v{};
    v.resize(n);
    for (int i = 0; i < n; i++) {
        std::memcpy(&v[i], reader(i), sizeof(T));
    }
    return v;
}

void load_meshes(IContext *context,
                 const std::filesystem::path &path,
                 const tinygltf::Model &gltf,
                 Model &model) {
    auto make_reader = [&](int accessor_index) {
        auto &accessor = gltf.accessors[accessor_index];
        auto &buffer_view = gltf.bufferViews[accessor.bufferView];
        auto stride = accessor.ByteStride(buffer_view);
        auto offset = accessor.byteOffset + buffer_view.byteOffset;
        auto &buffer = gltf.buffers[buffer_view.buffer];
        return [offset, &buffer, stride](int index) {
            return &buffer.data[offset + index * stride];
        };
    };

    model.meshes.reserve(gltf.meshes.size());
    for (auto &gltf_mesh : gltf.meshes) {
        Model::Mesh mesh{};
        mesh.name = gltf_mesh.name;

        auto &primitives = mesh.primitives;
        primitives.reserve(gltf_mesh.primitives.size());

        for (auto &gltf_prim : gltf_mesh.primitives) {
            auto visit_attr = [&](const std::string &attr_name,
                                  auto &&func,
                                  int accessor_type,
                                  bool report_warn) {
                auto it = gltf_prim.attributes.find(attr_name);
                if (it == gltf_prim.attributes.end()) {
                    return;
                }
                auto accessor_index = it->second;
                if (accessor_index < 0) {
                    return;
                }
                auto warn = [&](const std::string &info) {
                    if (report_warn) {
                        gltf_warn(path, info);
                    }
                };
                auto accessor = gltf.accessors[accessor_index];
                if (accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    warn("Only support float vertex attribute");
                    return;
                }
                if (accessor.type != accessor_type) {
                    warn("Accessor type not support for attribute " +
                         attr_name);
                    return;
                }
                auto reader = make_reader(accessor_index);
                func(reader, accessor.count);
            };

            constexpr const char *ATTR_NAME_POSITION = "POSITION";
            constexpr const char *ATTR_NAME_NORMAL = "NORMAL";
            constexpr const char *ATTR_NAME_TANGENT = "TANGENT";
            constexpr const char *ATTR_NAME_TEX_COORD_0 = "TEXCOORD_0";

            auto visit_all_attrs = [&](auto &&func, bool report_warn) {
                auto visit = [&](const std::string &attr, int accessor_type) {
                    visit_attr(
                        attr,
                        [&](auto &&reader, auto &&num) {
                            func(attr, reader, num);
                        },
                        accessor_type,
                        report_warn);
                };

                // for all attributes see
                // https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md
                // we only visit what we need
                visit(ATTR_NAME_POSITION, TINYGLTF_TYPE_VEC3);
                visit(ATTR_NAME_NORMAL, TINYGLTF_TYPE_VEC3);
                visit(ATTR_NAME_TANGENT, TINYGLTF_TYPE_VEC4);
                visit(ATTR_NAME_TEX_COORD_0, TINYGLTF_TYPE_VEC2);
            };

            MeshInfo info{};

            visit_all_attrs(
                [&](auto &&attr, auto &&reader, size_t num) {
                    info.vertex_capacity = std::max(info.vertex_capacity, num);
                },
                true);

            if (gltf_prim.indices >= 0) {
                info.triangle_capacity =
                    (gltf.accessors[gltf_prim.indices].count + 2) / 3;
            }

            auto m = context->create_mesh(info);

            visit_all_attrs(
                [&](const std::string &attr, auto &&reader, size_t num) {
                    if (attr == ATTR_NAME_POSITION) {
                        auto v = read_to_vec<glm::vec3>(reader, num);
                        m->set_position(v.data(), 0, num);
                    }
                    if (attr == ATTR_NAME_NORMAL) {
                        auto v = read_to_vec<glm::vec3>(reader, num);
                        m->set_normal(v.data(), 0, num);
                    }
                    if (attr == ATTR_NAME_TANGENT) {
                        auto v = read_to_vec<glm::vec4>(reader, num);
                        m->set_tangent(v.data(), 0, num);
                    }
                    if (attr == ATTR_NAME_TEX_COORD_0) {
                        auto v = read_to_vec<glm::vec2>(reader, num);
                        m->set_tex_coord(v.data(), 0, num);
                    }
                },
                false);

            std::vector<uint32_t> indices;
            {
                auto accessor_index = gltf_prim.indices;
                if (accessor_index >= 0) {
                    auto &accessor = gltf.accessors[accessor_index];
                    auto reader = make_reader(accessor_index);
                    indices.resize(accessor.count);
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
                            throw std::runtime_error(
                                "invalid type for indices");
                        }
                    }
                }
            }
            while (indices.size() % 3 != 0) {
                indices.push_back(0);
            }
            m->set_indices(reinterpret_cast<glm::u32vec3 *>(indices.data()),
                           0,
                           indices.size() / 3);

            Model::Primitive prim{};
            prim.mesh = std::move(m);
            if (gltf_prim.material >= 0) {
                prim.material = gltf_prim.material;
            }
            primitives.emplace_back(std::move(prim));
        }

        model.meshes.emplace_back(std::move(mesh));
    }
}

void load_nodes(const tinygltf::Model &gltf, Model &model) {
    model.nodes.reserve(gltf.nodes.size());
    for (auto &gltf_node : gltf.nodes) {
        Model::Node node{};

        node.name = gltf_node.name;
        if (gltf_node.camera >= 0) {
            node.camera = gltf_node.camera;
        }
        if (gltf_node.mesh >= 0) {
            node.mesh = gltf_node.mesh;
        }
        node.children.reserve(gltf_node.children.size());
        for (auto child : gltf_node.children) {
            node.children.push_back(child);
        }

        auto transform = glm::identity<glm::mat4>();
        if (gltf_node.scale.size() == 3) {
            transform =
                glm::scale(transform,
                           glm::vec3(static_cast<float>(gltf_node.scale[0]),
                                     static_cast<float>(gltf_node.scale[1]),
                                     static_cast<float>(gltf_node.scale[2])));
        }
        if (gltf_node.rotation.size() == 4) {
            glm::quat rotation(static_cast<float>(gltf_node.rotation[0]),
                               static_cast<float>(gltf_node.rotation[1]),
                               static_cast<float>(gltf_node.rotation[2]),
                               static_cast<float>(gltf_node.rotation[3]));
            transform = glm::mat4_cast(rotation) * transform;
        }
        if (gltf_node.translation.size() == 3) {
            transform = glm::translate(
                transform,
                glm::vec3(static_cast<float>(gltf_node.translation[0]),
                          static_cast<float>(gltf_node.translation[1]),
                          static_cast<float>(gltf_node.translation[2])));
        }
        if (gltf_node.matrix.size() == 16) {
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    transform[i][j] =
                        static_cast<float>(gltf_node.matrix[i * 4 + j]);
                }
            }
        }

        node.local_to_parent = math::XformTRS<float>(transform);

        model.nodes.emplace_back(std::move(node));
    }
}

void load_scenes(const tinygltf::Model &gltf, Model &model) {
    if (gltf.defaultScene >= 0) {
        model.default_scene = gltf.defaultScene;
    }

    model.scenes.reserve(gltf.scenes.size());
    for (auto &gltf_scene : gltf.scenes) {
        Model::Scene scene{};

        scene.name = gltf_scene.name;
        scene.nodes.reserve(gltf_scene.nodes.size());
        for (auto node : gltf_scene.nodes) {
            scene.nodes.push_back(node);
        }

        model.scenes.emplace_back(std::move(scene));
    }
}

void load_cameras(const std::filesystem::path &path,
                  const tinygltf::Model &gltf,
                  Model &model) {
    model.cameras.reserve(gltf.cameras.size());
    for (auto &gltf_camera : gltf.cameras) {
        Model::Camera camera{};

        camera.name = gltf_camera.name;

        if (gltf_camera.type == "perspective") {
            Model::Camera::PerspectiveData pers{};
            const auto &gltf_pers = gltf_camera.perspective;

            pers.has_z_far = gltf_pers.zfar > 0.0;
            pers.y_fov = static_cast<float>(gltf_pers.yfov);
            pers.z_near = static_cast<float>(gltf_pers.znear);
            pers.z_far = static_cast<float>(gltf_pers.zfar);

            camera.data = pers;
        } else if (gltf_camera.type == "orthographic") {
            Model::Camera::OrthographicData ortho{};
            const auto &gltf_ortho = gltf_camera.orthographic;

            ortho.z_far = static_cast<float>(gltf_ortho.zfar);
            ortho.z_near = static_cast<float>(gltf_ortho.znear);
            ortho.y_mag = static_cast<float>(gltf_ortho.ymag);

            camera.data = ortho;
        } else {
            Model::Camera::PerspectiveData pers{};

            pers.has_z_far = true;
            pers.y_fov = glm::radians(45.0f);
            pers.z_near = 0.1f;
            pers.z_far = 100.0f;

            camera.data = pers;

            std::stringstream ss;
            ss << "Invalid camera type " << std::quoted(gltf_camera.type)
               << " at index " << model.cameras.size()
               << ", use default perspective camera { y_fov = " << pers.y_fov
               << ", z_near = " << pers.z_near << ", z_far = " << pers.z_far
               << " }";

            gltf_warn(path, ss.str());
        }

        model.cameras.emplace_back(std::move(camera));
    }
}

std::shared_ptr<ITexture> load_texture(IContext *context,
                                       const std::filesystem::path &path,
                                       const tinygltf::Model &gltf,
                                       const tinygltf::Texture &gltf_tex,
                                       bool need_srgb) {
    auto &image = gltf.images[gltf_tex.source];
    int width = image.width;
    int height = image.height;
    int channels = image.component;
    if (image.bits != 8 ||
        image.pixel_type != TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
        std::stringstream ss;
        ss << "Only supports 8bit texture, ignore texture "
           << std::quoted(gltf_tex.name);
        gltf_warn(path, ss.str());
        return nullptr;
    }

    // TODO take sampler into account.
    auto texture = context->create_texture_2d(
        get_8_bit_texture_format(channels, need_srgb), width, height);

    texture->set_data((void *)image.image.data(),
                      width * height * channels,
                      0,
                      0,
                      0,
                      0,
                      0,
                      width,
                      height,
                      1);
    texture->generate_mipmap();

    return texture;
}

void load_textures(IContext *context,
                   const std::filesystem::path &path,
                   const tinygltf::Model &gltf,
                   Model &model) {
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

    model.textures.reserve(gltf.textures.size());
    for (int index = 0; index < gltf.textures.size(); index++) {
        const auto &gltf_tex = gltf.textures[index];

        Model::Texture tex{};

        tex.name = gltf_tex.name;
        tex.texture =
            load_texture(context, path, gltf, gltf_tex, need_srgb[index]);

        model.textures.emplace_back(std::move(tex));
    }
}

void load_materials(IContext *context,
                    const tinygltf::Model &gltf,
                    Model &model) {
    // TODO
}

Model load_gltf(IContext *context,
                const std::filesystem::path &path,
                const tinygltf::Model &gltf) {
    Model model{};

    load_meshes(context, path, gltf, model);
    load_nodes(gltf, model);
    load_scenes(gltf, model);
    load_cameras(path, gltf, model);
    load_textures(context, path, gltf, model);
    load_materials(context, gltf, model);

    return model;
}
} // namespace

Model load_gltf(IContext *context, const std::filesystem::path &path) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model gltf;
    std::string err;
    std::string warn;

    using namespace std::chrono;
    auto start = high_resolution_clock::now();

    auto extension = path.extension();
    auto model_path_str = path.string();

    auto gltf_error = [&](const std::string &info) {
        log_error("Failed to load " + model_path_str + ": " + info);
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
        return Model{};
    }

    if (!warn.empty()) {
        gltf_warn(path, warn);
    }

    if (!err.empty()) {
        gltf_error(err);
        return Model{};
    }

    if (!ret) {
        log_error("Failed to parse " + path.string());
        return Model{};
    }

    auto mid = high_resolution_clock::now();

    auto model = load_gltf(context, path, gltf);

    auto stop = high_resolution_clock::now();

    auto total_duration = duration_cast<milliseconds>(stop - start);
    auto decode_duration = duration_cast<milliseconds>(mid - start);
    auto upload_duration = duration_cast<milliseconds>(stop - mid);
    {
        std::stringstream ss;
        ss << "Load gltf file " << path << " takes " << total_duration.count()
           << "ms, tinygltf decode takes " << decode_duration.count()
           << "ms, upload takes " << upload_duration.count() << "ms";
        log_info(ss.str());
    }

    return model;
}
} // namespace ars::render