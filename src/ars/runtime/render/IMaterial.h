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

constexpr const char *RES_TYPE_NAME_MATERIAL = "ars::render::IMaterial";

enum class MaterialPropertyType { Texture, Int, Float, Float2, Float3, Float4 };

template <typename T> struct MaterialPropertyTypeTrait;

template <> struct MaterialPropertyTypeTrait<std::shared_ptr<ITexture>> {
    static constexpr MaterialPropertyType Type = MaterialPropertyType::Texture;
};

template <> struct MaterialPropertyTypeTrait<int> {
    static constexpr MaterialPropertyType Type = MaterialPropertyType::Int;
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
                                              int,
                                              float,
                                              glm::vec2,
                                              glm::vec3,
                                              glm::vec4> {
  private:
    using Base = std::variant<std::shared_ptr<ITexture>,
                              int,
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
    int id{};
    MaterialPropertyType type{};

    using Setter =
        std::function<void(IMaterial *, const MaterialPropertyVariant &)>;
    using Getter = std::function<MaterialPropertyVariant(IMaterial *)>;
    Setter setter{};
    Getter getter{};

    MaterialPropertyInfo() = default;
    MaterialPropertyInfo(const char *name,
                         MaterialPropertyType type,
                         Setter setter,
                         Getter getter);

    template <typename M, typename T>
    static MaterialPropertyInfo make(const char *name, T M::*mem) {
        using Type = std::remove_reference_t<T>;
        if constexpr (std::is_integral_v<Type> || std::is_enum_v<Type>) {
            return MaterialPropertyInfo(
                name,
                MaterialPropertyType::Int,
                [mem](IMaterial *m, const MaterialPropertyVariant &v) {
                    auto mat = dynamic_cast<M *>(m);
                    mat->*mem = static_cast<Type>(std::get<int>(v));
                },
                [mem](IMaterial *m) {
                    auto mat = dynamic_cast<M *>(m);
                    return static_cast<int>(mat->*mem);
                });
        } else {
            return MaterialPropertyInfo(
                name,
                MaterialPropertyTypeTrait<Type>::Type,
                [mem](IMaterial *m, const MaterialPropertyVariant &v) {
                    auto mat = dynamic_cast<M *>(m);
                    mat->*mem = std::get<Type>(v);
                },
                [mem](IMaterial *m) {
                    auto mat = dynamic_cast<M *>(m);
                    return mat->*mem;
                });
        }
    }
};

enum class MaterialType : uint32_t { Error, Unlit, MetallicRoughnessPBR };

enum class MaterialAlphaMode : uint32_t { Opaque, Blend };

// IMaterial prototype is owned by the context.
class IMaterialPrototype {
  public:
    IMaterialPrototype(MaterialType type,
                       std::vector<MaterialPropertyInfo> properties);

    virtual ~IMaterialPrototype() = default;

    virtual std::shared_ptr<IMaterial> create_material() = 0;

    [[nodiscard]] MaterialType type() const;
    [[nodiscard]] const std::vector<MaterialPropertyInfo> &properties() const;

  protected:
    MaterialType _type{};
    std::vector<MaterialPropertyInfo> _properties{};
};

// If the material does not have corresponding property, or the property
// type mismatches, setter will do nothing and getter will return default
// value.
class IMaterial : public IRes {
    RTTR_DERIVE(IRes);

  public:
    explicit IMaterial(IMaterialPrototype *prototype);

    std::string res_type() const override;

    static int str_to_id(const std::string &id);

    [[nodiscard]] MaterialType type() const;
    [[nodiscard]] IMaterialPrototype *prototype() const;

    template <typename T> void set(int id, T &&value) {
        using Type = std::remove_cv_t<std::remove_reference_t<T>>;
        if constexpr (std::is_enum_v<Type> || std::is_integral_v<Type>) {
            set_variant(id, static_cast<int>(value));
        } else {
            set_variant(id, std::forward<T>(value));
        }
    }

    template <typename T> void set(const std::string &name, T &&value) {
        set(str_to_id(name), std::forward<T>(value));
    }

    void set_variant(int id, const MaterialPropertyVariant &value);
    std::optional<MaterialPropertyVariant> get_variant(int id);

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
