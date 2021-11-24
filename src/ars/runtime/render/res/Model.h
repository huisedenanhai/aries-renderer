#pragma once

#include "../IMesh.h"
#include "../IScene.h"
#include <ars/runtime/core/ResData.h>
#include <ars/runtime/core/Serde.h>
#include <ars/runtime/core/math/Transform.h>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ars::render {
class IContext;
class ITexture;

struct Model {
    using Index = uint32_t;

    struct Node {
        std::string name;
        std::vector<Index> children{};
        std::optional<Index> mesh;
        std::optional<Index> camera;
        std::optional<Index> light;
        math::XformTRS<float> local_to_parent{};
    };

    struct Primitive {
        std::shared_ptr<IMesh> mesh;
        std::optional<Index> material;
    };

    struct Mesh {
        std::string name;
        std::vector<Primitive> primitives;
    };

    struct Scene {
        std::string name;
        std::vector<Index> nodes;
    };

    struct Camera {
        std::string name;
        CameraData data;
    };

    struct Texture {
        std::string name;
        std::shared_ptr<ITexture> texture;
    };

    struct Material {
        std::string name;
        std::shared_ptr<IMaterial> material;
    };

    enum LightType { Directional, Point, Spot };

    struct Light {
        std::string name;
        LightType type;
        glm::vec3 color;
        float intensity;
    };

    std::optional<Index> default_scene{};
    std::vector<Node> nodes{};
    std::vector<Mesh> meshes{};
    std::vector<Scene> scenes{};
    std::vector<Camera> cameras{};
    std::vector<Texture> textures{};
    std::vector<Material> materials{};
    std::vector<Light> lights{};
};

Model load_gltf(IContext *context, const std::filesystem::path &path);

constexpr const char *MESH_RES_TYPE_NAME = "ars::render::IMesh";
constexpr const char *MATERIAL_RES_TYPE_NAME = "ars::render::IMaterial";

struct MeshResMeta {
    math::AABB<float> aabb;
    DataSlice position;
    DataSlice normal;
    DataSlice tangent;
    DataSlice tex_coord;
    DataSlice indices;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(
        MeshResMeta, aabb, position, normal, tangent, tex_coord, indices)
};

std::shared_ptr<IMesh> load_mesh(IContext *context, const ResData &data);

struct MaterialResMeta {
    MaterialType type = MaterialType::Error;
    DataSlice properties;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(MaterialResMeta, type, properties)
};

std::shared_ptr<IMaterial> load_material(IContext *context,
                                         const ResData &data);
} // namespace ars::render