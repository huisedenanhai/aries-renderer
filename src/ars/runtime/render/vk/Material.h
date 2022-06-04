#pragma once

#include "../IMaterial.h"
#include "Texture.h"
#include "Vulkan.h"
#include "features/Renderer.h"

namespace ars::render::vk {
class Context;
class GraphicsPipeline;

struct MaterialPropertyBlockInfo {
    std::string name;
    std::vector<MaterialPropertyInfo> properties{};

    [[nodiscard]] std::optional<uint32_t>
    find_property_index(const std::string &prop_name) const;

    MaterialPropertyBlockInfo &
    add_property(const std::string &prop_name,
                 MaterialPropertyType type,
                 const MaterialPropertyVariant &default_value);

    template <typename T>
    MaterialPropertyBlockInfo &add_property(const std::string &prop_name,
                                            T &&default_value) {
        using Type = std::remove_cv_t<std::remove_reference_t<T>>;
        return add_property(prop_name,
                            MaterialPropertyTypeTrait<Type>::Type,
                            std::forward<T>(default_value));
    }

    [[nodiscard]] uint32_t texture_count() const;

    [[nodiscard]] std::string to_glsl() const;
};

class MaterialPropertyBlockLayout {
  public:
    MaterialPropertyBlockLayout(Context *context,
                                MaterialPropertyBlockInfo info);
    [[nodiscard]] const MaterialPropertyBlockInfo &info() const;
    [[nodiscard]] uint32_t property_offset(uint32_t index) const;
    [[nodiscard]] uint32_t data_block_size() const;
    [[nodiscard]] Context *context() const;

  private:
    void init_data_block_layout();

    Context *_context{};
    MaterialPropertyBlockInfo _info{};
    std::vector<uint32_t> _property_offsets{};
    uint32_t _data_block_size{};
};

class MaterialPropertyBlock {
  public:
    explicit MaterialPropertyBlock(
        std::shared_ptr<MaterialPropertyBlockLayout> layout);

    [[nodiscard]] std::shared_ptr<MaterialPropertyBlockLayout> layout() const;

    // If the property with name is not found, return std::nullopt.
    // If the property is found and is not set, return default value.
    // For texture, if the value is nullptr, return default value.
    // If default texture is nullptr, return default white texture.
    std::optional<MaterialPropertyVariant> get_variant(const std::string &name);

    // If the property with name is not found, do nothing
    // If the property is found but type mismatch, do nothing
    void set_variant(const std::string &name,
                     const MaterialPropertyVariant &value);

    std::vector<Handle<Texture>> referenced_textures();
    void fill_data(void *ptr);

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

  private:
    // Not null
    std::shared_ptr<ITexture> get_texture_by_index(uint32_t index);
    MaterialPropertyVariant get_variant_by_index(uint32_t index);
    void set_variant_by_index(uint32_t index,
                              const MaterialPropertyVariant &value);

    std::shared_ptr<MaterialPropertyBlockLayout> _layout = nullptr;
    std::vector<uint8_t> _data_block{};
    std::vector<std::shared_ptr<ITexture>> _texture_owners{};
};

struct MaterialPass {
    GraphicsPipeline *pipeline = nullptr;
    MaterialPropertyBlock *property_block = nullptr;
};

struct MaterialPassInfo {
    RenderPassID pass_id = {};
    bool skinned = false;

    [[nodiscard]] uint32_t encode() const;

    constexpr static uint32_t MAX_INDEX = RenderPassID_Count * 2;
};

struct MaterialPrototype;

class Material : public IMaterial {
  public:
    Material(const MaterialInfo &info, MaterialPrototype *prototype);

    void set_variant(const std::string &name,
                     const MaterialPropertyVariant &value) override;
    std::optional<MaterialPropertyVariant>
    get_variant(const std::string &name) override;

    std::vector<MaterialPropertyInfo> properties() override;
    MaterialInfo info() override;

    MaterialPass pass(const MaterialPassInfo &info);

  private:
    MaterialInfo _info{};
    std::unique_ptr<MaterialPropertyBlock> _property_block{};
    MaterialPrototype *_prototype = nullptr;
};

std::shared_ptr<Material> upcast(const std::shared_ptr<IMaterial> &m);

struct MaterialPrototype {
    MaterialInfo info{};
    std::shared_ptr<MaterialPropertyBlockLayout> property_layout{};
    std::array<std::shared_ptr<GraphicsPipeline>, MaterialPassInfo::MAX_INDEX>
        passes{};

    std::shared_ptr<Material> create();
};

class MaterialFactory {
  public:
    explicit MaterialFactory(Context *context);

    std::shared_ptr<Material> create_material(const MaterialInfo &info);
    std::shared_ptr<Material> default_material();

  private:
    void init_default_material();

    Context *_context{};
    std::map<MaterialInfo, std::unique_ptr<MaterialPrototype>>
        _material_prototypes{};
    std::shared_ptr<Material> _default_material{};
};
} // namespace ars::render::vk