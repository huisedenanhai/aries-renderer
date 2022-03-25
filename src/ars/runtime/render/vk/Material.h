#pragma once

#include "../IMaterial.h"
#include "Texture.h"
#include "Vulkan.h"
#include "features/Renderer.h"

namespace ars::render::vk {
class Context;
class GraphicsPipeline;

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