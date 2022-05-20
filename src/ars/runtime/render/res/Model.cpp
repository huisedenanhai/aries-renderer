#include "Model.h"
#include "../IMaterial.h"
#include "../IMesh.h"
#include "Texture.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/misc/Visitor.h>
#include <chrono>
#include <mikktspace/mikktspace.h>
#include <sstream>
#include <tiny_gltf.h>

namespace ars::render {
namespace {
void gltf_warn(const std::filesystem::path &path, const std::string &info) {
    ARS_LOG_WARN("Loading {}: {}", path.string(), info);
}

template <typename T, typename R>
R cast_value(const unsigned char *ptr, bool normalized) {
    auto r = static_cast<R>(*reinterpret_cast<const T *>(ptr));
    if constexpr (std::is_floating_point_v<R> && std::is_integral_v<T>) {
        if (normalized) {
            auto max_value = std::numeric_limits<T>::max();
            return r / static_cast<R>(max_value);
        }
    }
    return r;
}

template <typename R>
R cast_value(const unsigned char *ptr,
             int gltf_component_type,
             bool normalized) {
    switch (gltf_component_type) {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
        return cast_value<int8_t, R>(ptr, normalized);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        return cast_value<uint8_t, R>(ptr, normalized);
    case TINYGLTF_COMPONENT_TYPE_SHORT:
        return cast_value<int16_t, R>(ptr, normalized);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        return cast_value<uint16_t, R>(ptr, normalized);
    case TINYGLTF_COMPONENT_TYPE_INT:
        return cast_value<int32_t, R>(ptr, normalized);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        return cast_value<uint32_t, R>(ptr, normalized);
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
        return cast_value<float, R>(ptr, normalized);
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
        return cast_value<double, R>(ptr, normalized);
    default:
        break;
    }
    return {};
}

std::string name_or_index(const std::string &name, int index) {
    return name.empty() ? std::to_string(index) : name;
}

std::string gltf_mesh_path(const std::filesystem::path &path,
                           const tinygltf::Model &gltf,
                           int mesh_index,
                           int prim_index) {
    if (mesh_index < 0 || prim_index < 0) {
        return "";
    }
    return canonical_res_path(
        path / "meshes" /
        fmt::format("{}.{}",
                    name_or_index(gltf.meshes[mesh_index].name, mesh_index),
                    prim_index));
}

std::string gltf_texture_path(const std::filesystem::path &path,
                              const tinygltf::Model &gltf,
                              int index) {
    if (index < 0) {
        return "";
    }
    auto image = gltf.textures[index].source;
    return canonical_res_path(path.parent_path() / gltf.images[image].uri);
}

std::string gltf_material_path(const std::filesystem::path &path,
                               const tinygltf::Model &gltf,
                               int mat_index) {
    if (mat_index < 0) {
        return "";
    }
    return canonical_res_path(
        path / "materials" /
        name_or_index(gltf.materials[mat_index].name, mat_index));
}

template <typename T> constexpr int get_gltf_component_type() {
    if constexpr (std::is_same_v<T, int8_t>) {
        return TINYGLTF_COMPONENT_TYPE_BYTE;
    }
    if constexpr (std::is_same_v<T, uint8_t>) {
        return TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE;
    }
    if constexpr (std::is_same_v<T, int16_t>) {
        return TINYGLTF_COMPONENT_TYPE_SHORT;
    }
    if constexpr (std::is_same_v<T, uint16_t>) {
        return TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
    }
    if constexpr (std::is_same_v<T, int32_t>) {
        return TINYGLTF_COMPONENT_TYPE_INT;
    }
    if constexpr (std::is_same_v<T, uint32_t>) {
        return TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
    }
    if constexpr (std::is_same_v<T, float>) {
        return TINYGLTF_COMPONENT_TYPE_FLOAT;
    }
    if constexpr (std::is_same_v<T, double>) {
        return TINYGLTF_COMPONENT_TYPE_DOUBLE;
    }

    return 0;
}

size_t gltf_component_type_size(int gltf_component_type) {
    switch (gltf_component_type) {
    case TINYGLTF_COMPONENT_TYPE_BYTE:
        return sizeof(int8_t);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
        return sizeof(uint8_t);
    case TINYGLTF_COMPONENT_TYPE_SHORT:
        return sizeof(int16_t);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
        return sizeof(uint16_t);
    case TINYGLTF_COMPONENT_TYPE_INT:
        return sizeof(int32_t);
    case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
        return sizeof(uint32_t);
    case TINYGLTF_COMPONENT_TYPE_FLOAT:
        return sizeof(float);
    case TINYGLTF_COMPONENT_TYPE_DOUBLE:
        return sizeof(double);
    default:
        break;
    }
    return 0;
}

size_t gltf_type_component_count(int type) {
    switch (type) {
    case TINYGLTF_TYPE_VEC2:
        return 2;
    case TINYGLTF_TYPE_VEC3:
        return 3;
    case TINYGLTF_TYPE_VEC4:
        return 4;
    case TINYGLTF_TYPE_MAT2:
        return 4;
    case TINYGLTF_TYPE_MAT3:
        return 9;
    case TINYGLTF_TYPE_MAT4:
        return 16;
    case TINYGLTF_TYPE_SCALAR:
        return 1;
    default:
        break;
    }
    return 0;
}

template <typename T>
std::vector<T> read_buffer_to_vector(const tinygltf::Model &gltf,
                                     int accessor_index,
                                     int gltf_expected_type = 0,
                                     T default_value = {}) {
    auto &accessor = gltf.accessors[accessor_index];
    auto &buffer_view = gltf.bufferViews[accessor.bufferView];
    auto stride = accessor.ByteStride(buffer_view);
    auto offset = accessor.byteOffset + buffer_view.byteOffset;
    auto &buffer = gltf.buffers[buffer_view.buffer];

    auto component_count = gltf_type_component_count(accessor.type);
    auto component_size = gltf_component_type_size(accessor.componentType);
    auto expected_component_count =
        gltf_expected_type != 0 ? gltf_type_component_count(gltf_expected_type)
                                : component_count;

    constexpr auto target_component_type = get_gltf_component_type<T>();
    static_assert(target_component_type != 0);
    auto tight_packed = stride == (component_size * component_count);
    auto need_cast = target_component_type != accessor.componentType;

    std::vector<T> result{};
    result.resize(expected_component_count * accessor.count);

    auto available_component_count =
        std::min(component_count, expected_component_count);
    auto pad_element = [&](int i) {
        for (int j = static_cast<int>(available_component_count);
             j < expected_component_count;
             j++) {
            result[i * expected_component_count + j] = default_value;
        }
    };

    if (tight_packed && !need_cast &&
        expected_component_count == component_count) {
        std::memcpy(result.data(),
                    &buffer.data[offset],
                    component_size * expected_component_count * accessor.count);
    } else if (!need_cast) {
        for (int i = 0; i < accessor.count; i++) {
            std::memcpy(&result[i * expected_component_count],
                        &buffer.data[offset + i * stride],
                        available_component_count * component_size);
            pad_element(i);
        }
    } else {
        for (int i = 0; i < accessor.count; i++) {
            for (int j = 0; j < available_component_count; j++) {
                result[i * expected_component_count + j] = cast_value<T>(
                    &buffer.data[offset + i * stride + j * component_size],
                    accessor.componentType,
                    accessor.normalized);
            }
            pad_element(i);
        }
    }

    return result;
}

template <typename T>
void read_buffer_to_vector(std::vector<T> &result,
                           const tinygltf::Model &gltf,
                           int accessor_index,
                           int gltf_expected_type = 0,
                           T default_value = {}) {
    result = read_buffer_to_vector<T>(
        gltf, accessor_index, gltf_expected_type, default_value);
}

struct PrimitiveData {
    std::vector<float> position{};
    std::vector<float> normal{};
    std::vector<float> tangent{};
    std::vector<float> tex_coord{};
    std::vector<uint32_t> joint{};
    std::vector<float> weight{};
    std::vector<uint32_t> indices{};

    size_t triangle_count() const {
        return indices.size() / 3;
    }

    size_t vertex_count() const {
        return position.size() / 3;
    }

    void generate_default_tex_coord() {
        // Initialize with all zeros
        tex_coord.clear();
        tex_coord.resize(vertex_count() * 2);
    }

    uint32_t vertex_index(int face, int vert) const {
        return indices[face * 3 + vert];
    }

    void generate_default_tangent() {
        // Reserve space for tangent data.
        tangent.clear();
        tangent.resize(vertex_count() * 4);

        SMikkTSpaceInterface inter{};
        inter.m_getNumFaces = [](const SMikkTSpaceContext *ctx) {
            auto data = reinterpret_cast<PrimitiveData *>(ctx->m_pUserData);
            return static_cast<int>(data->triangle_count());
        };
        inter.m_getNumVerticesOfFace = [](const SMikkTSpaceContext *ctx,
                                          int face) { return 3; };
        inter.m_getPosition = [](const SMikkTSpaceContext *ctx,
                                 float fvPosOut[],
                                 const int iFace,
                                 const int iVert) {
            auto data = reinterpret_cast<PrimitiveData *>(ctx->m_pUserData);
            auto vert = data->vertex_index(iFace, iVert);
            std::memcpy(fvPosOut, &data->position[vert * 3], 3 * sizeof(float));
        };
        inter.m_getNormal = [](const SMikkTSpaceContext *ctx,
                               float fvNormOut[],
                               const int iFace,
                               const int iVert) {
            auto data = reinterpret_cast<PrimitiveData *>(ctx->m_pUserData);
            auto vert = data->vertex_index(iFace, iVert);
            std::memcpy(fvNormOut, &data->normal[vert * 3], 3 * sizeof(float));
        };
        inter.m_getTexCoord = [](const SMikkTSpaceContext *ctx,
                                 float fvTexcOut[],
                                 const int iFace,
                                 const int iVert) {
            auto data = reinterpret_cast<PrimitiveData *>(ctx->m_pUserData);
            auto vert = data->vertex_index(iFace, iVert);
            std::memcpy(
                fvTexcOut, &data->tex_coord[vert * 2], 2 * sizeof(float));
        };
        inter.m_setTSpaceBasic = [](const SMikkTSpaceContext *ctx,
                                    const float fvTangent[],
                                    const float fSign,
                                    const int iFace,
                                    const int iVert) {
            auto data = reinterpret_cast<PrimitiveData *>(ctx->m_pUserData);
            auto vert = data->vertex_index(iFace, iVert);
            auto idx = vert * 4;
            std::memcpy(&data->tangent[idx], fvTangent, 3 * sizeof(float));
            data->tangent[idx + 3] = fSign;
        };

        SMikkTSpaceContext ctx{};
        ctx.m_pInterface = &inter;
        ctx.m_pUserData = this;
        genTangSpaceDefault(&ctx);
    }

    void canonicalize() {
        if (tex_coord.empty()) {
            generate_default_tex_coord();
        }

        if (tangent.empty()) {
            generate_default_tangent();
        }

        while (indices.size() % 3 != 0) {
            indices.push_back(0);
        }
    }

    std::shared_ptr<IMesh> upload_mesh(IContext *context) {
        MeshInfo info{};

        info.vertex_capacity = vertex_count();
        if (!indices.empty()) {
            info.triangle_capacity = triangle_count();
        }
        info.skinned = !joint.empty() && !weight.empty();

        auto m = context->create_mesh(info);

        auto aabb = math::AABB<float>::from_points(
            reinterpret_cast<glm::vec3 *>(&position[0]),
            reinterpret_cast<glm::vec3 *>(&position[0]) + info.vertex_capacity);
        m->set_aabb(aabb);
        m->set_position(reinterpret_cast<glm::vec3 *>(position.data()),
                        0,
                        info.vertex_capacity);
        m->set_normal(reinterpret_cast<glm::vec3 *>(normal.data()),
                      0,
                      info.vertex_capacity);
        m->set_tangent(reinterpret_cast<glm::vec4 *>(tangent.data()),
                       0,
                       info.vertex_capacity);
        m->set_tex_coord(reinterpret_cast<glm::vec2 *>(tex_coord.data()),
                         0,
                         info.vertex_capacity);
        if (info.skinned) {
            m->set_joint(reinterpret_cast<glm::uvec4 *>(joint.data()),
                         0,
                         info.vertex_capacity);
            m->set_weight(reinterpret_cast<glm::vec4 *>(weight.data()),
                          0,
                          info.vertex_capacity);
        }

        m->set_indices(reinterpret_cast<glm::u32vec3 *>(indices.data()),
                       0,
                       info.triangle_capacity);
        m->set_triangle_count(info.triangle_capacity);
        m->update_acceleration_structure();

        return m;
    }
};

PrimitiveData gltf_read_primitive_data(const tinygltf::Model &gltf,
                                       const tinygltf::Primitive &gltf_prim) {
    PrimitiveData data{};
    constexpr const char *ATTR_NAME_POSITION = "POSITION";
    constexpr const char *ATTR_NAME_NORMAL = "NORMAL";
    constexpr const char *ATTR_NAME_TANGENT = "TANGENT";
    constexpr const char *ATTR_NAME_TEX_COORD_0 = "TEXCOORD_0";
    constexpr const char *ATTR_NAME_JOINTS_0 = "JOINTS_0";
    constexpr const char *ATTR_NAME_WEIGHTS_0 = "WEIGHTS_0";

    auto read_attr = [&](const char *name,
                         auto &v,
                         int gltf_expected_type,
                         const auto &default_value) {
        auto it = gltf_prim.attributes.find(name);
        if (it == gltf_prim.attributes.end()) {
            return;
        }
        auto accessor_index = it->second;
        if (accessor_index < 0) {
            return;
        }
        read_buffer_to_vector(
            v, gltf, accessor_index, gltf_expected_type, default_value);
    };

    read_attr("POSITION", data.position, TINYGLTF_TYPE_VEC3, 0.0f);
    read_attr("NORMAL", data.normal, TINYGLTF_TYPE_VEC3, 0.0f);
    read_attr("TANGENT", data.tangent, TINYGLTF_TYPE_VEC4, 0.0f);
    read_attr("TEXCOORD_0", data.tex_coord, TINYGLTF_TYPE_VEC2, 0.0f);
    read_attr(
        "JOINTS_0", data.joint, TINYGLTF_TYPE_VEC4, static_cast<uint32_t>(0));
    read_attr("WEIGHTS_0", data.weight, TINYGLTF_TYPE_VEC4, 0.0f);

    if (gltf_prim.indices >= 0) {
        data.indices = read_buffer_to_vector<uint32_t>(gltf, gltf_prim.indices);
    }

    data.canonicalize();
    return data;
}

void load_meshes(IContext *context,
                 const std::filesystem::path &path,
                 const tinygltf::Model &gltf,
                 Model &model) {
    model.meshes.reserve(gltf.meshes.size());
    for (int mesh_index = 0; mesh_index < gltf.meshes.size(); mesh_index++) {
        auto &gltf_mesh = gltf.meshes[mesh_index];
        Model::Mesh mesh{};
        mesh.name = gltf_mesh.name;

        auto &primitives = mesh.primitives;
        primitives.reserve(gltf_mesh.primitives.size());
        for (int prim_index = 0; prim_index < gltf_mesh.primitives.size();
             prim_index++) {
            auto &gltf_prim = gltf_mesh.primitives[prim_index];
            auto prim_data = gltf_read_primitive_data(gltf, gltf_prim);
            auto m = prim_data.upload_mesh(context);
            m->set_path(gltf_mesh_path(path, gltf, mesh_index, prim_index));

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
        if (gltf_node.skin >= 0) {
            node.skin = gltf_node.skin;
        }
        auto gltf_light_ext = gltf_node.extensions.find("KHR_lights_punctual");
        if (gltf_light_ext != gltf_node.extensions.end()) {
            if (gltf_light_ext->second.Has("light")) {
                node.light = gltf_light_ext->second.Get("light").Get<int>();
            }
        }

        node.children.reserve(gltf_node.children.size());
        for (auto child : gltf_node.children) {
            node.children.push_back(child);
        }

        math::XformTRS<float> trs{};
        if (gltf_node.translation.size() == 3) {
            trs.set_translation(
                glm::vec3(static_cast<float>(gltf_node.translation[0]),
                          static_cast<float>(gltf_node.translation[1]),
                          static_cast<float>(gltf_node.translation[2])));
        }

        if (gltf_node.rotation.size() == 4) {
            // GLTF quaternion component order [x, y, z, w]
            // glm quaternion constructor component order [w, x, y, z]
            glm::quat rotation(static_cast<float>(gltf_node.rotation[3]),
                               static_cast<float>(gltf_node.rotation[0]),
                               static_cast<float>(gltf_node.rotation[1]),
                               static_cast<float>(gltf_node.rotation[2]));
            trs.set_rotation(rotation);
        }

        if (gltf_node.scale.size() == 3) {
            trs.set_scale(glm::vec3(static_cast<float>(gltf_node.scale[0]),
                                    static_cast<float>(gltf_node.scale[1]),
                                    static_cast<float>(gltf_node.scale[2])));
        }

        if (gltf_node.matrix.size() == 16) {
            auto matrix = glm::identity<glm::mat4>();
            for (int i = 0; i < 4; i++) {
                for (int j = 0; j < 4; j++) {
                    matrix[i][j] =
                        static_cast<float>(gltf_node.matrix[i * 4 + j]);
                }
            }
            trs = math::XformTRS<float>(matrix);
        }

        node.local_to_parent = trs;

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
            Perspective pers{};
            const auto &gltf_pers = gltf_camera.perspective;

            pers.y_fov = static_cast<float>(gltf_pers.yfov);
            pers.z_near = static_cast<float>(gltf_pers.znear);
            pers.z_far = static_cast<float>(gltf_pers.zfar);
            if (pers.z_far == 0.0f) {
                pers.z_far = 10000.0f;
                std::stringstream ss;
                ss << "Infinite Z Far is not supported, use default z far "
                   << pers.z_far;
                gltf_warn(path, ss.str());
            }

            camera.data = pers;
        } else if (gltf_camera.type == "orthographic") {
            Orthographic ortho{};
            const auto &gltf_ortho = gltf_camera.orthographic;

            ortho.z_far = static_cast<float>(gltf_ortho.zfar);
            ortho.z_near = static_cast<float>(gltf_ortho.znear);
            ortho.y_mag = static_cast<float>(gltf_ortho.ymag);

            camera.data = ortho;
        } else {
            Perspective pers{};

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
                                       int tex_index,
                                       bool need_srgb) {
    const auto &gltf_tex = gltf.textures[tex_index];
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

    std::filesystem::path tex_path = gltf_texture_path(path, gltf, tex_index);

    TextureInfo info{};
    info.name = tex_path.filename().string();
    info.type = TextureType::Texture2D;
    info.format = get_8_bit_texture_format(channels, need_srgb);
    info.width = width;
    info.height = height;

    if (gltf_tex.sampler >= 0) {
        auto &sampler = gltf.samplers[gltf_tex.sampler];

        info.min_filter = gltf_translate_filter_mode(sampler.minFilter);
        info.mag_filter = gltf_translate_filter_mode(sampler.magFilter);
        info.mipmap_mode = gltf_translate_mipmap_mode(sampler.minFilter);
        info.wrap_u = gltf_translate_wrap_mode(sampler.wrapS);
        info.wrap_v = gltf_translate_wrap_mode(sampler.wrapT);
        // tinygltf mark wrapR as unused, assign a default value here.
        info.wrap_w = WrapMode::Repeat;
    }

    auto texture = context->create_texture(info);

    texture->set_path(tex_path.string());
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
            load_texture(context, path, gltf, index, need_srgb[index]);

        model.textures.emplace_back(std::move(tex));
    }
}

void load_materials(IContext *context,
                    const std::filesystem::path &path,
                    const tinygltf::Model &gltf,
                    Model &model) {
    auto tex = [&](int index) {
        return index < 0 ? nullptr : model.textures[index].texture;
    };

    model.materials.reserve(gltf.materials.size());
    for (int mat_index = 0; mat_index < gltf.materials.size(); mat_index++) {
        auto &gltf_mat = gltf.materials[mat_index];
        Model::Material mat{};

        mat.name = gltf_mat.name;
        MaterialInfo mat_info{};
        mat_info.shading_model = MaterialShadingModel::MetallicRoughnessPBR;

        if (gltf_mat.doubleSided) {
            mat_info.features |= MaterialFeature_DoubleSidedBit;
        }
        if (gltf_mat.alphaMode == "MASK") {
            mat_info.features |= MaterialFeature_AlphaClipBit;
        }

        auto &m = mat.material = context->create_material(mat_info);
        auto &pbr = gltf_mat.pbrMetallicRoughness;

        m->set_path(gltf_material_path(path, gltf, mat_index));
        m->set("base_color_tex", tex(pbr.baseColorTexture.index));
        m->set("base_color_factor",
               glm::vec4(pbr.baseColorFactor[0],
                         pbr.baseColorFactor[1],
                         pbr.baseColorFactor[2],
                         pbr.baseColorFactor[3]));
        m->set("metallic_factor", (float)pbr.metallicFactor);
        m->set("roughness_factor", (float)pbr.roughnessFactor);
        m->set("metallic_roughness_tex",
               tex(pbr.metallicRoughnessTexture.index));
        m->set("normal_tex", tex(gltf_mat.normalTexture.index));
        m->set("normal_scale", (float)gltf_mat.normalTexture.scale);
        m->set("occlusion_tex", tex(gltf_mat.occlusionTexture.index));
        m->set("occlusion_strength", (float)gltf_mat.occlusionTexture.strength);
        m->set("emission_tex", tex(gltf_mat.emissiveTexture.index));
        m->set("emission_factor",
               glm::vec3(gltf_mat.emissiveFactor[0],
                         gltf_mat.emissiveFactor[1],
                         gltf_mat.emissiveFactor[2]));

        if (mat_info.features & MaterialFeature_AlphaClipBit) {
            m->set("alpha_cutoff", (float)gltf_mat.alphaCutoff);
        }

        model.materials.emplace_back(std::move(mat));
    }
}

Model::LightType translate_light_type(const std::string &type) {
    if (type == "point") {
        return Model::Point;
    }
    if (type == "directional") {
        return Model::Directional;
    }
    assert(type == "spot");
    return Model::Spot;
}

void load_lights(const tinygltf::Model &gltf, Model &model) {
    model.lights.reserve(gltf.lights.size());
    for (auto &gltf_light : gltf.lights) {
        Model::Light light{};

        light.name = gltf_light.name;
        light.color = glm::vec3(
            gltf_light.color[0], gltf_light.color[1], gltf_light.color[2]);
        light.intensity = static_cast<float>(gltf_light.intensity);
        light.type = translate_light_type(gltf_light.type);

        model.lights.emplace_back(std::move(light));
    }
}

void load_skins(const tinygltf::Model &gltf, Model &model) {
    model.skins.reserve(gltf.skins.size());
    for (auto &gltf_skin : gltf.skins) {
        Model::Skin skin{};

        skin.name = gltf_skin.name;
        auto joint_count = gltf_skin.joints.size();
        skin.joints.reserve(joint_count);
        for (int i = 0; i < joint_count; i++) {
            skin.joints.push_back(gltf_skin.joints[i]);
        }

        skin.inverse_binding_matrices.resize(joint_count);
        if (gltf_skin.inverseBindMatrices < 0) {
            std::fill(skin.inverse_binding_matrices.begin(),
                      skin.inverse_binding_matrices.end(),
                      glm::identity<glm::mat4>());
        } else {
            auto ibm_data = read_buffer_to_vector<float>(
                gltf, gltf_skin.inverseBindMatrices);
            assert(ibm_data.size() == joint_count * 16);
            auto ibm_ptr = reinterpret_cast<glm::mat4 *>(ibm_data.data());
            std::copy(ibm_ptr,
                      ibm_ptr + joint_count,
                      skin.inverse_binding_matrices.begin());
        }

        model.skins.emplace_back(std::move(skin));
    }
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
    load_materials(context, path, gltf, model);
    load_lights(gltf, model);
    load_skins(gltf, model);

    return model;
}
} // namespace

WrapMode gltf_translate_wrap_mode(int value) {
    switch (value) {
    case TINYGLTF_TEXTURE_WRAP_REPEAT:
        return WrapMode::Repeat;
    case TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE:
        return WrapMode::ClampToEdge;
    case TINYGLTF_TEXTURE_WRAP_MIRRORED_REPEAT:
        return WrapMode::MirroredRepeat;
    default:
        break;
    }
    return WrapMode::Repeat;
}

FilterMode gltf_translate_filter_mode(int filter) {
    switch (filter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
        return FilterMode::Nearest;
    case TINYGLTF_TEXTURE_FILTER_LINEAR:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
        return FilterMode::Linear;
    default:
        break;
    }
    return FilterMode::Linear;
}

MipmapMode gltf_translate_mipmap_mode(int filter) {
    switch (filter) {
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
        return MipmapMode::Nearest;
    case TINYGLTF_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
    case TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
        return MipmapMode::Linear;
    default:
        break;
    }
    return MipmapMode::Linear;
}

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
        ARS_LOG_ERROR("Failed to parse {}", path.string());
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
        ARS_LOG_INFO("Load gltf file {} takes {}ms, tinygltf decode takes "
                     "{}ms, upload takes {}ms",
                     path.string(),
                     total_duration.count(),
                     decode_duration.count(),
                     upload_duration.count());
    }

    return model;
}

} // namespace ars::render
