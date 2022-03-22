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
    explicit MaterialPrototype(Context *context,
                               const MaterialPropertyBlockInfo &info);

    std::shared_ptr<IMaterial> create_material() override;
    MaterialPropertyBlockInfo info() override;

    std::shared_ptr<MaterialPropertyBlockLayout> property_block_layout() const;
    [[nodiscard]] Context *context() const;

  private:
    Context *_context = nullptr;
    std::shared_ptr<MaterialPropertyBlockLayout> _block_layout = nullptr;
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

    MaterialPropertyBlock *property_block() const;

    MaterialPrototype *prototype_vk() const;

  private:
    MaterialPrototype *_prototype = nullptr;
    std::unique_ptr<MaterialPropertyBlock> _property_block{};
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