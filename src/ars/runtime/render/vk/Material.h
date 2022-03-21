#pragma once

#include "../IMaterial.h"
#include "Texture.h"
#include "Vulkan.h"

namespace ars::render::vk {
class Context;

class MaterialPass {
  public:
};

class MaterialPrototype : public IMaterialPrototype {
  public:
    explicit MaterialPrototype(Context *context, MaterialPrototypeInfo info);

    std::shared_ptr<IMaterial> create_material() override;
    MaterialPrototypeInfo info() override;

    [[nodiscard]] Context *context() const;
    uint32_t property_offset(uint32_t index) const;
    uint32_t data_block_size() const;

  private:
    void init_data_block_layout();

    Context *_context = nullptr;
    MaterialPrototypeInfo _info{};
    std::vector<uint32_t> _property_offsets{};
    uint32_t _data_block_size{};
};

MaterialPrototype *upcast(IMaterialPrototype *prototype);

class Material : public IMaterial {
  public:
    explicit Material(MaterialPrototype *prototype);

    IMaterialPrototype *prototype() override;
    void set_variant(const std::string &name,
                     const MaterialPropertyVariant &value) override;
    std::optional<MaterialPropertyVariant>
    get_variant(const std::string &name) override;

    MaterialPrototype *prototype_vk() const;
    std::vector<Handle<Texture>> referenced_textures();

  private:
    std::shared_ptr<ITexture> get_texture_by_index(uint32_t index);
    MaterialPropertyVariant get_variant_by_index(uint32_t index);
    void set_variant_by_index(uint32_t index,
                              const MaterialPropertyVariant &value);

    MaterialPrototype *_prototype = nullptr;
    std::vector<uint8_t> _data_block{};
    std::vector<std::shared_ptr<ITexture>> _texture_owners{};
};

std::shared_ptr<Material> upcast(const std::shared_ptr<IMaterial> &m);

class MaterialPrototypeRegistry {
  public:
    explicit MaterialPrototypeRegistry(Context *context);
    ~MaterialPrototypeRegistry() = default;

    [[nodiscard]] IMaterialPrototype *prototype(MaterialType type) const;

  private:
    std::unique_ptr<MaterialPrototype> _unlit_material_prototype{};
    std::unique_ptr<MaterialPrototype> _metallic_roughness_material_prototype{};
};
} // namespace ars::render::vk