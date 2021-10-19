#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ars::render {
class ITexture;
class IMaterial;

enum class MaterialPropertyType { Texture, Int, Float, Float2, Float3, Float4 };

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

    MaterialPropertyType type() const;
};

struct MaterialPropertyInfo {
    std::string name{};
    int id{};
    MaterialPropertyType type{};
    std::function<void(IMaterial *, const MaterialPropertyVariant &)> setter{};
    std::function<MaterialPropertyVariant(IMaterial *)> getter{};

    MaterialPropertyInfo() = default;
    MaterialPropertyInfo(const char *name, MaterialPropertyType type);
};

enum class MaterialType { Error, Unlit, MetallicRoughnessPBR };

enum class MaterialAlphaMode { Opaque, Blend };

// IMaterial prototype is owned by the context.
class IMaterialPrototype {
  public:
    IMaterialPrototype(MaterialType type,
                       std::vector<MaterialPropertyInfo> properties);

    virtual ~IMaterialPrototype() = default;

    virtual std::shared_ptr<IMaterial> create_material() = 0;

    MaterialType type() const;
    const std::vector<MaterialPropertyInfo> &properties() const;

  protected:
    MaterialType _type{};
    std::vector<MaterialPropertyInfo> _properties{};
};

// If the material does not have corresponding property, or the property
// type mismatches, setter will do nothing and getter will return default
// value.
class IMaterial {
  public:
    explicit IMaterial(IMaterialPrototype *prototype);

    virtual ~IMaterial() = default;

    static int str_to_id(const std::string &id);

    MaterialType type() const;
    IMaterialPrototype *prototype() const;

    template <typename T> void set(int id, T &&value) {
        using Type = std::remove_cv_t<std::remove_reference_t<T>>;
        if constexpr (std::is_enum_v<Type>) {
            set_variant(id, static_cast<int>(value));
        } else if constexpr (std::is_same_v<Type, bool>) {
            set_variant(id, value ? 1 : 0);
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
} // namespace ars::render
