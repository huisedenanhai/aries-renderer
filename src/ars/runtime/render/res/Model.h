#pragma once

#include "../IScene.h"
#include <ars/runtime/core/math/Transform.h>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ars::render {
class IMesh;
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
} // namespace ars::render