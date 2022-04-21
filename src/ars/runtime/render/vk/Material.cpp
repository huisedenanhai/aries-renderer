#include "Material.h"
#include "Context.h"
#include "features/Drawer.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/misc/Visitor.h>
#include <sstream>

namespace ars::render::vk {
std::optional<uint32_t> MaterialPropertyBlockInfo::find_property_index(
    const std::string &prop_name) const {
    for (int i = 0; i < properties.size(); i++) {
        if (properties[i].name == prop_name) {
            return i;
        }
    }
    return std::nullopt;
}

MaterialPropertyBlockInfo &MaterialPropertyBlockInfo::add_property(
    const std::string &prop_name,
    MaterialPropertyType type,
    const MaterialPropertyVariant &default_value) {
    MaterialPropertyInfo p{};
    p.name = prop_name;
    p.type = type;
    p.default_value = default_value;
    properties.push_back(p);
    return *this;
}

std::string MaterialPropertyBlockInfo::compile_glsl() const {
    std::stringstream ss;
    ss << "struct " << name << " {\n";
    for (auto &p : properties) {
        const char *ty_name = "float";
        switch (p.type) {
        case MaterialPropertyType::Texture:
            ty_name = "int";
            break;
        case MaterialPropertyType::Float:
            ty_name = "float";
            break;
        case MaterialPropertyType::Float2:
            ty_name = "vec2";
            break;
        case MaterialPropertyType::Float3:
            ty_name = "vec3";
            break;
        case MaterialPropertyType::Float4:
            ty_name = "vec4";
            break;
        }
        ss << fmt::format("    {} {};\n", ty_name, p.name);
    }

    ss << "};";
    return ss.str();
}

MaterialPropertyBlockLayout::MaterialPropertyBlockLayout(
    Context *context, MaterialPropertyBlockInfo info)
    : _context(context), _info(std::move(info)) {
    init_data_block_layout();
}

namespace {
void property_size_and_alignment(MaterialPropertyType type,
                                 uint32_t &size,
                                 uint32_t &alignment) {
    switch (type) {
    case MaterialPropertyType::Texture:
        size = sizeof(uint32_t);
        alignment = sizeof(uint32_t);
        break;
    case MaterialPropertyType::Float:
        size = sizeof(float);
        alignment = sizeof(float);
        break;
    case MaterialPropertyType::Float2:
        size = sizeof(float) * 2;
        alignment = sizeof(float) * 2;
        break;
    case MaterialPropertyType::Float3:
        size = sizeof(float) * 3;
        alignment = sizeof(float) * 4;
        break;
    case MaterialPropertyType::Float4:
        size = sizeof(float) * 4;
        alignment = sizeof(float) * 4;
        break;
    }
}
} // namespace

void MaterialPropertyBlockLayout::init_data_block_layout() {
    _property_offsets.reserve(_info.properties.size());

    uint32_t cur_ptr = 0;
    uint32_t max_align = 1;

    auto align_ptr = [](uint32_t ptr, uint32_t align) {
        assert(align > 0);
        return ((ptr + align - 1) / align) * align;
    };

    for (auto &p : _info.properties) {
        uint32_t prop_size, prop_align;
        property_size_and_alignment(p.type, prop_size, prop_align);
        cur_ptr = align_ptr(cur_ptr, prop_align);
        max_align = std::max(max_align, prop_align);
        _property_offsets.push_back(cur_ptr);
        cur_ptr += prop_size;
    }
    cur_ptr = align_ptr(cur_ptr, max_align);

    _data_block_size = cur_ptr;
}

uint32_t MaterialPropertyBlockLayout::property_offset(uint32_t index) const {
    assert(index < _property_offsets.size());
    return _property_offsets[index];
}

uint32_t MaterialPropertyBlockLayout::data_block_size() const {
    return _data_block_size;
}

const MaterialPropertyBlockInfo &MaterialPropertyBlockLayout::info() const {
    return _info;
}

Context *MaterialPropertyBlockLayout::context() const {
    return _context;
}

MaterialPropertyBlock::MaterialPropertyBlock(
    std::shared_ptr<MaterialPropertyBlockLayout> layout)
    : _layout(std::move(layout)) {
    assert(_layout != nullptr);

    auto &info = _layout->info();
    _texture_owners.resize(info.properties.size());
    _data_block.resize(_layout->data_block_size());

    // Init with default value
    for (int i = 0; i < info.properties.size(); i++) {
        set_variant_by_index(i, info.properties[i].default_value);
    }
}

void MaterialPropertyBlock::set_variant(const std::string &name,
                                        const MaterialPropertyVariant &value) {
    auto &info = _layout->info();
    auto index = info.find_property_index(name);
    if (!index.has_value()) {
        ARS_LOG_ERROR(
            "Material \"{}\" does not have property \"{}\"", info.name, name);
        return;
    }

    set_variant_by_index(*index, value);
}

std::optional<MaterialPropertyVariant>
MaterialPropertyBlock::get_variant(const std::string &name) {
    auto &info = _layout->info();
    auto index = info.find_property_index(name);
    if (!index.has_value()) {
        return std::nullopt;
    }
    return get_variant_by_index(*index);
}

std::shared_ptr<MaterialPropertyBlockLayout>
MaterialPropertyBlock::layout() const {
    return _layout;
}

void MaterialPropertyBlock::set_variant_by_index(
    uint32_t index, const MaterialPropertyVariant &value) {
    auto &info = _layout->info();
    assert(index < info.properties.size());

    auto &p = info.properties[index];
    if (value.type() != p.type) {
        ARS_LOG_ERROR("Property \"{}\" of material \"{}\" type mismatch",
                      p.name,
                      info.name);
        return;
    }

    auto data_ptr = &_data_block[_layout->property_offset(index)];
    std::visit(ars::make_visitor([&](auto &&v) {
                   using T =
                       std::remove_cv_t<std::remove_reference_t<decltype(v)>>;
                   if constexpr (std::is_same_v<T, std::shared_ptr<ITexture>>) {
                       _texture_owners[index] = v;
                   } else {
                       *reinterpret_cast<T *>(data_ptr) = v;
                   }
               }),
               value);
}

// TODO bindless on supported platform, currently the textures are simply
// returned in their property order, and the shader use index of that tex in the
// returned vector to access them
std::vector<Handle<Texture>> MaterialPropertyBlock::referenced_textures() {
    std::vector<Handle<Texture>> textures{};
    auto props = _layout->info().properties;
    for (int i = 0; i < props.size(); i++) {
        if (props[i].type == MaterialPropertyType::Texture) {
            textures.push_back(upcast(get_texture_by_index(i).get()));
        }
    }
    return textures;
}

void MaterialPropertyBlock::fill_data(void *ptr) {
    std::memcpy(ptr, _data_block.data(), _data_block.size());

    auto props = _layout->info().properties;
    uint32_t tex_id = 0;
    for (int i = 0; i < props.size(); i++) {
        if (props[i].type == MaterialPropertyType::Texture) {
            auto offset = _layout->property_offset(i);
            auto mem_ptr =
                reinterpret_cast<uint32_t *>((uint8_t *)(ptr) + offset);
            *mem_ptr = tex_id++;
        }
    }
}

MaterialPropertyVariant
MaterialPropertyBlock::get_variant_by_index(uint32_t index) {
    auto &info = layout()->info();
    assert(index < info.properties.size());

    auto &p = info.properties[index];
    auto data_ptr = &_data_block[_layout->property_offset(index)];
    switch (p.type) {
    case MaterialPropertyType::Texture:
        return get_texture_by_index(index);
    case MaterialPropertyType::Float:
        return *reinterpret_cast<float *>(data_ptr);
    case MaterialPropertyType::Float2:
        return *reinterpret_cast<glm::vec2 *>(data_ptr);
    case MaterialPropertyType::Float3:
        return *reinterpret_cast<glm::vec3 *>(data_ptr);
    case MaterialPropertyType::Float4:
        return *reinterpret_cast<glm::vec4 *>(data_ptr);
    }
}

std::shared_ptr<ITexture>
MaterialPropertyBlock::get_texture_by_index(uint32_t index) {
    auto &info = _layout->info();
    assert(index < info.properties.size());
    auto &p = info.properties[index];
    assert(p.type == MaterialPropertyType::Texture);
    auto tex = _texture_owners[index];
    if (tex == nullptr) {
        if (std::holds_alternative<std::shared_ptr<ITexture>>(
                p.default_value)) {
            tex = std::get<std::shared_ptr<ITexture>>(p.default_value);
        }
    }
    if (tex == nullptr) {
        tex = _layout->context()->default_texture(DefaultTexture::White);
    }
    return tex;
}

MaterialFactory::MaterialFactory(Context *context) : _context(context) {
    init_unlit_template();
    init_metallic_roughness_template();
    init_default_material();
}

std::shared_ptr<Material> MaterialFactory::create_material(MaterialType type) {
    switch (type) {
    case MaterialType::Unlit:
        return _unlit_template.create();
    case MaterialType::MetallicRoughnessPBR:
        return _metallic_roughness_template.create();
    }
    return _metallic_roughness_template.create();
}

void MaterialFactory::init_unlit_template() {
    auto white_tex = _context->default_texture(DefaultTexture::White);
    MaterialPropertyBlockInfo unlit{};
    unlit.name = "Unlit";
    unlit.add_property("color_factor", glm::vec4(1.0f));
    unlit.add_property("color_tex", white_tex);

    _unlit_template.type = MaterialType::Unlit;
    _unlit_template.property_layout =
        std::make_shared<MaterialPropertyBlockLayout>(_context, unlit);

    auto &geom_pass = _unlit_template.passes[RenderPassID_Geometry];
    geom_pass.property_layout = _unlit_template.property_layout;
    // TODO init pipeline
}

void MaterialFactory::init_metallic_roughness_template() {
    auto white_tex = _context->default_texture(DefaultTexture::White);

    MaterialPropertyBlockInfo pbr{};
    pbr.name = "MetallicRoughnessLit";
    pbr.add_property("base_color_factor", glm::vec4(1.0f));
    pbr.add_property("metallic_factor", 1.0f);
    pbr.add_property("roughness_factor", 1.0f);
    pbr.add_property("normal_scale", 1.0f);
    pbr.add_property("occlusion_strength", 1.0f);
    pbr.add_property("emission_factor", glm::vec3(0.0f));
    pbr.add_property("base_color_tex", white_tex);
    pbr.add_property("metallic_roughness_tex", white_tex);
    pbr.add_property("normal_tex",
                     _context->default_texture(DefaultTexture::Normal));
    pbr.add_property("occlusion_tex", white_tex);
    pbr.add_property("emission_tex", white_tex);

    _metallic_roughness_template.type = MaterialType::MetallicRoughnessPBR;
    _metallic_roughness_template.property_layout =
        std::make_shared<MaterialPropertyBlockLayout>(_context, pbr);

    // Geometry pass
    {
        auto vert_shader =
            Shader::find_precompiled(_context,
                                     "Draw/MetallicRoughnessPBR.glsl",
                                     {"FRILL_SHADER_STAGE_VERT"});
        auto frag_shader =
            Shader::find_precompiled(_context,
                                     "Draw/MetallicRoughnessPBR.glsl",
                                     {"FRILL_SHADER_STAGE_FRAG"});

        auto &geom_pass =
            _metallic_roughness_template.passes[RenderPassID_Geometry];
        geom_pass.property_layout =
            _metallic_roughness_template.property_layout;
        geom_pass.pipeline = create_pipeline(
            RenderPassID_Geometry, {vert_shader.get(), frag_shader.get()});
    }

    // Shadow pass
    {
        auto vert_shader =
            Shader::find_precompiled(_context, "Draw/Depth.vert");

        VkPipelineRasterizationStateCreateInfo raster{
            VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
        raster.cullMode = VK_CULL_MODE_NONE;
        raster.lineWidth = 1.0f;
        raster.polygonMode = VK_POLYGON_MODE_FILL;
        raster.depthBiasEnable = VK_TRUE;

        auto &shadow_pass =
            _metallic_roughness_template.passes[RenderPassID_Shadow];
        shadow_pass.property_layout = nullptr;
        shadow_pass.pipeline =
            create_pipeline(RenderPassID_Shadow, {vert_shader.get()}, &raster);
    }
}

std::shared_ptr<GraphicsPipeline> MaterialFactory::create_pipeline(
    RenderPassID id,
    const std::vector<Shader *> &shaders,
    VkPipelineRasterizationStateCreateInfo *raster) {

    VkPipelineVertexInputStateCreateInfo vertex_input{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkVertexInputBindingDescription vert_bindings[5] = {
        {0,
         static_cast<uint32_t>(sizeof(InstanceDrawParam)),
         VK_VERTEX_INPUT_RATE_INSTANCE},
        {1,
         static_cast<uint32_t>(sizeof(glm::vec3)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {2,
         static_cast<uint32_t>(sizeof(glm::vec3)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {3,
         static_cast<uint32_t>(sizeof(glm::vec4)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {4,
         static_cast<uint32_t>(sizeof(glm::vec2)),
         VK_VERTEX_INPUT_RATE_VERTEX},
    };

    VkVertexInputAttributeDescription vert_attrs[5] = {
        {0, 0, VK_FORMAT_R32_UINT, offsetof(InstanceDrawParam, instance_id)},
        {1, 1, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {2, 2, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {3, 3, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
        {4, 4, VK_FORMAT_R32G32_SFLOAT, 0},
    };

    vertex_input.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(std::size(vert_attrs));
    vertex_input.pVertexAttributeDescriptions = vert_attrs;
    vertex_input.vertexBindingDescriptionCount =
        static_cast<uint32_t>(std::size(vert_bindings));
    vertex_input.pVertexBindingDescriptions = vert_bindings;

    auto depth_stencil = enabled_depth_stencil_state();

    GraphicsPipelineInfo info{};
    info.shaders = shaders;
    info.subpass = _context->renderer_data()->subpass(id);

    info.vertex_input = &vertex_input;
    info.depth_stencil = &depth_stencil;
    info.raster = raster;

    return std::make_shared<GraphicsPipeline>(_context, info);
}

std::shared_ptr<Material> MaterialFactory::default_material() {
    return _default_material;
}

void MaterialFactory::init_default_material() {
    _default_material = create_material(MaterialType::MetallicRoughnessPBR);
    _default_material->set("base_color_factor",
                           glm::vec4(0.8f, 0.8f, 0.8f, 1.0f));
    _default_material->set("metallic_factor", 0.0f);
}

void Material::set_variant(const std::string &name,
                           const MaterialPropertyVariant &value) {
    if (_property_block != nullptr) {
        _property_block->set_variant(name, value);
    }
    // Also set properties with same name for passes
    for (auto &p : _passes) {
        if (p.property_block == nullptr) {
            continue;
        }
        if (p.property_block->layout()
                ->info()
                .find_property_index(name)
                .has_value()) {
            p.property_block->set(name, value);
        }
    }
}

std::optional<MaterialPropertyVariant>
Material::get_variant(const std::string &name) {
    if (_property_block == nullptr) {
        return std::nullopt;
    }
    return _property_block->get_variant(name);
}

Material::Material(
    MaterialType type,
    const std::shared_ptr<MaterialPropertyBlockLayout> &property_layout,
    MaterialPassArray passes)
    : _type(type), _passes(std::move(passes)) {
    if (property_layout != nullptr) {
        _property_block =
            std::make_unique<MaterialPropertyBlock>(property_layout);
    }
}

std::vector<MaterialPropertyInfo> Material::properties() {
    if (_property_block == nullptr) {
        return {};
    }
    return _property_block->layout()->info().properties;
}

MaterialType Material::type() {
    return _type;
}

MaterialPass Material::pass(const MaterialPassInfo &info) {
    auto id = static_cast<uint32_t>(info.pass_id);
    assert(id < _passes.size());

    MaterialPass p{};
    p.property_block = _passes[id].property_block.get();
    p.pipeline = _passes[id].pipeline.get();

    return p;
}

std::shared_ptr<Material> upcast(const std::shared_ptr<IMaterial> &m) {
    return std::dynamic_pointer_cast<Material>(m);
}

MaterialPassOwned MaterialPassTemplate::create() {
    MaterialPassOwned p{};

    if (property_layout != nullptr) {
        p.property_block =
            std::make_unique<MaterialPropertyBlock>(property_layout);
    }
    p.pipeline = pipeline;

    return p;
}

std::shared_ptr<Material> MaterialTemplate::create() {
    MaterialPassArray ps{};
    for (int i = 0; i < RenderPassID_Count; i++) {
        ps[i] = passes[i].create();
    }

    return std::make_shared<Material>(type, property_layout, std::move(ps));
}
} // namespace ars::render::vk