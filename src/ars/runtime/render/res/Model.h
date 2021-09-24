#pragma once

#include <ars/runtime/core/math/Transform.h>
#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ars::render {
class IMesh;
class IMaterial;
class IContext;

struct Model {
    using Index = uint32_t;

    struct Node {
        std::string name;
        std::vector<Index> children{};
        std::optional<Index> mesh;
        std::optional<Index> camera;
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
        struct PerspectiveData {
            float y_fov;
            bool has_z_far;
            float z_far;
            float z_near;
        };

        struct OrthographicData {
            float y_mag;
            float z_far;
            float z_near;
        };

        enum Type { Perspective, Orthographic };

        std::string name;

        std::variant<PerspectiveData, OrthographicData> data;

        [[nodiscard]] Type type() const;
        [[nodiscard]] glm::mat4 projection_matrix(float aspect) const;
    };

    struct Texture {
        std::string name;
        std::shared_ptr<ITexture> texture;
    };

    struct Material {
        std::string name;
        std::shared_ptr<IMaterial> material;
    };

    std::optional<Index> default_scene{};
    std::vector<Node> nodes{};
    std::vector<Mesh> meshes{};
    std::vector<Scene> scenes{};
    std::vector<Camera> cameras{};
    std::vector<Texture> textures{};
    std::vector<Material> materials{};
};

Model load_gltf(IContext *context, const std::filesystem::path &path);

} // namespace ars::render