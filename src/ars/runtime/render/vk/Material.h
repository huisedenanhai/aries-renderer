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

    std::string compile_glsl() const;
};

class MaterialPropertyBlockLayout {
  public:
    MaterialPropertyBlockLayout(IContext *context,
                                MaterialPropertyBlockInfo info);
    const MaterialPropertyBlockInfo &info() const;
    uint32_t property_offset(uint32_t index) const;
    uint32_t data_block_size() const;
    IContext *context() const;

  private:
    void init_data_block_layout();

    IContext *_context{};
    MaterialPropertyBlockInfo _info{};
    std::vector<uint32_t> _property_offsets{};
    uint32_t _data_block_size{};
};

class MaterialPropertyBlock {
  public:
    explicit MaterialPropertyBlock(
        std::shared_ptr<MaterialPropertyBlockLayout> layout);

    std::shared_ptr<MaterialPropertyBlockLayout> layout() const;

    // If the property with name is not found, return std::nullopt.
    // If the property is found and is not set, return default value.
    // For texture, if the value is nullptr, return default value.
    // If default texture is nullptr, return default white texture.
    std::optional<MaterialPropertyVariant> get_variant(const std::string &name);

    // If the property with name is not found, do nothing
    // If the property is found but type mismatch, do nothing
    void set_variant(const std::string &name,
                     const MaterialPropertyVariant &value);

    std::vector<std::shared_ptr<ITexture>> referenced_textures();

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

struct MaterialPassOwned {
    std::shared_ptr<GraphicsPipeline> pipeline = nullptr;
    // Can be null if no property is required
    std::unique_ptr<MaterialPropertyBlock> property_block = nullptr;
};

using MaterialPassArray = std::array<MaterialPassOwned, RenderPassID_Count>;

class Material : public IMaterial {
  public:
    Material(
        MaterialType type,
        const std::shared_ptr<MaterialPropertyBlockLayout> &property_layout,
        MaterialPassArray passes);

    void set_variant(const std::string &name,
                     const MaterialPropertyVariant &value) override;
    std::optional<MaterialPropertyVariant>
    get_variant(const std::string &name) override;

    std::vector<MaterialPropertyInfo> properties() override;
    MaterialType type() override;

    MaterialPass pass(RenderPassID pass_id);

  private:
    MaterialType _type{};
    std::unique_ptr<MaterialPropertyBlock> _property_block{};
    MaterialPassArray _passes{};
};

std::shared_ptr<Material> upcast(const std::shared_ptr<IMaterial> &m);

struct MaterialPassTemplate {
    std::shared_ptr<MaterialPropertyBlockLayout> property_layout{};
    std::shared_ptr<GraphicsPipeline> pipeline{};

    MaterialPassOwned create();
};

struct MaterialTemplate {
    MaterialType type{};
    std::shared_ptr<MaterialPropertyBlockLayout> property_layout{};
    std::array<MaterialPassTemplate, RenderPassID_Count> passes{};

    std::shared_ptr<Material> create();
};

class MaterialFactory {
  public:
    explicit MaterialFactory(Context *context);

    std::shared_ptr<Material> create_material(MaterialType type);

  private:
    void init_unlit_template();
    void init_metallic_roughness_template();
    std::shared_ptr<GraphicsPipeline>
    create_pipeline(RenderPassID id, const std::string &glsl_file);

    Context *_context{};
    MaterialTemplate _unlit_template{};
    MaterialTemplate _metallic_roughness_template{};
};
} // namespace ars::render::vk