#pragma once

#include <ars/runtime/core/Reflect.h>
#include <ars/runtime/core/Res.h>
#include <functional>
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace ars::render {
class ITexture;
class IMaterial;

enum class MaterialPropertyType { Texture, Float, Float2, Float3, Float4 };

template <typename T> struct MaterialPropertyTypeTrait;

template <> struct MaterialPropertyTypeTrait<std::shared_ptr<ITexture>> {
    static constexpr MaterialPropertyType Type = MaterialPropertyType::Texture;
};

template <> struct MaterialPropertyTypeTrait<float> {
    static constexpr MaterialPropertyType Type = MaterialPropertyType::Float;
};

template <> struct MaterialPropertyTypeTrait<glm::vec2> {
    static constexpr MaterialPropertyType Type = MaterialPropertyType::Float2;
};

template <> struct MaterialPropertyTypeTrait<glm::vec3> {
    static constexpr MaterialPropertyType Type = MaterialPropertyType::Float3;
};

template <> struct MaterialPropertyTypeTrait<glm::vec4> {
    static constexpr MaterialPropertyType Type = MaterialPropertyType::Float4;
};

struct MaterialPropertyVariant : std::variant<std::shared_ptr<ITexture>,
                                              float,
                                              glm::vec2,
                                              glm::vec3,
                                              glm::vec4> {
  private:
    using Base = std::variant<std::shared_ptr<ITexture>,
                              float,
                              glm::vec2,
                              glm::vec3,
                              glm::vec4>;

  public:
    using Base::Base;

    [[nodiscard]] MaterialPropertyType type() const;
};

struct MaterialPropertyInfo {
    std::string name{};
    MaterialPropertyType type{};
    MaterialPropertyVariant default_value{};
};

enum class MaterialType : uint32_t { Unlit, MetallicRoughnessPBR };

struct MaterialPrototypeInfo {
    std::string name;
    MaterialType shading_model = MaterialType::MetallicRoughnessPBR;
    std::vector<MaterialPropertyInfo> properties{};

    [[nodiscard]] std::optional<uint32_t>
    find_property_index(const std::string &prop_name);

    MaterialPrototypeInfo &
    add_property(const std::string &prop_name,
                 MaterialPropertyType type,
                 const MaterialPropertyVariant &default_value);

    template <typename T>
    MaterialPrototypeInfo &add_property(const std::string &prop_name,
                                        T &&default_value) {
        using Type = std::remove_cv_t<std::remove_reference_t<T>>;
        return add_property(prop_name,
                            MaterialPropertyTypeTrait<Type>::Type,
                            std::forward<T>(default_value));
    }
};

// IMaterial prototype is owned by the context.
class IMaterialPrototype {
  public:
    explicit IMaterialPrototype(MaterialPrototypeInfo info);

    virtual ~IMaterialPrototype() = default;

    virtual std::shared_ptr<IMaterial> create_material() = 0;

    MaterialPrototypeInfo info() const;

  protected:
    MaterialPrototypeInfo _info{};
};

class IMaterial : public IRes {
    RTTR_DERIVE(IRes);

  public:
    explicit IMaterial(IMaterialPrototype *prototype);

    [[nodiscard]] IMaterialPrototype *prototype() const;

    template <typename T> void set(const std::string &name, T &&value) {
        set_variant(name, std::forward<T>(value));
    }

    template <typename T> std::optional<T> get(const std::string &name) {
        auto v = get_variant(name);
        if (v.has_value() && std::holds_alternative<T>(*v)) {
            return std::get<T>(*v);
        }
        return std::nullopt;
    }

    // If the property with name is not found, do nothing
    // If the property is found but type mismatch, do nothing
    virtual void set_variant(const std::string &name,
                             const MaterialPropertyVariant &value) = 0;

    // If the property with name is not found, return std::nullopt.
    // If the property is found and is not set, return default value.
    // For texture, if the value is nullptr, return default value.
    //
    // For backend, when rendering, if a texture property has no value or
    // default value, it should use the default white texture. Other types use
    // default zeroed values.
    virtual std::optional<MaterialPropertyVariant>
    get_variant(const std::string &name) = 0;

    static void register_type();

  protected:
    IMaterialPrototype *_prototype = nullptr;
};

enum class ShaderKind {
    Vertex,
    Fragment,
    TessControl,
    TessEvaluation,
    Geometry,
    Compute,
    RayGen,
    AnyHit,
    ClosestHit,
    Miss,
    Intersection,
    Callable,
    Task,
    Mesh
};

enum class ShaderOptimizationLevel { Zero, Size, Performance };

struct ShaderCompileOptions {
    std::unordered_map<std::string, std::string> macro_defs;
    ShaderOptimizationLevel optimization = ShaderOptimizationLevel::Zero;
};

std::vector<uint8_t> glsl_to_spirv(const std::string &glsl,
                                   ShaderKind default_kind,
                                   const char *name,
                                   const ShaderCompileOptions &options = {});
} // namespace ars::render
