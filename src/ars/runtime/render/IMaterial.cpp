#include "IMaterial.h"
#include "IContext.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/misc/Visitor.h>
#include <rttr/registration>
#include <shaderc/shaderc.hpp>
#include <sstream>
#include <utility>

namespace ars::render {
void IMaterial::register_type() {
    rttr::registration::class_<IMaterial>("ars::render::IMaterial");
}

MaterialPropertyType MaterialPropertyVariant::type() const {
    return std::visit(
        [](auto &&v) {
            using T = std::remove_cv_t<std::remove_reference_t<decltype(v)>>;
            return MaterialPropertyTypeTrait<T>::Type;
        },
        *this);
}

namespace {
shaderc_shader_kind get_shaderc_kind(ShaderKind kind) {
    switch (kind) {
    case ShaderKind::Vertex:
        return shaderc_glsl_default_vertex_shader;
    case ShaderKind::Fragment:
        return shaderc_glsl_default_fragment_shader;
    case ShaderKind::TessControl:
        return shaderc_glsl_default_tess_control_shader;
    case ShaderKind::TessEvaluation:
        return shaderc_glsl_default_tess_evaluation_shader;
    case ShaderKind::Geometry:
        return shaderc_glsl_default_geometry_shader;
    case ShaderKind::Compute:
        return shaderc_glsl_default_compute_shader;
    case ShaderKind::RayGen:
        return shaderc_glsl_default_raygen_shader;
    case ShaderKind::AnyHit:
        return shaderc_glsl_default_anyhit_shader;
    case ShaderKind::ClosestHit:
        return shaderc_glsl_default_closesthit_shader;
    case ShaderKind::Miss:
        return shaderc_glsl_default_miss_shader;
    case ShaderKind::Intersection:
        return shaderc_glsl_default_intersection_shader;
    case ShaderKind::Callable:
        return shaderc_glsl_default_callable_shader;
    case ShaderKind::Task:
        return shaderc_glsl_default_task_shader;
    case ShaderKind::Mesh:
        return shaderc_glsl_default_mesh_shader;
    }
    // Make compiler happy
    return shaderc_glsl_default_vertex_shader;
}

shaderc_optimization_level
get_shaderc_optimization_level(ShaderOptimizationLevel level) {
    switch (level) {
    case ShaderOptimizationLevel::Zero:
        return shaderc_optimization_level_zero;
    case ShaderOptimizationLevel::Size:
        return shaderc_optimization_level_size;
    case ShaderOptimizationLevel::Performance:
        return shaderc_optimization_level_performance;
    }
    // Make compiler happy
    return shaderc_optimization_level_zero;
}
} // namespace

std::vector<uint8_t> glsl_to_spirv(const std::string &glsl,
                                   ShaderKind default_kind,
                                   const char *name,
                                   const ShaderCompileOptions &options) {
    shaderc::Compiler compiler{};
    shaderc::CompileOptions shaderc_options{};
    for (auto &[k, v] : options.macro_defs) {
        shaderc_options.AddMacroDefinition(k, v);
    }

    auto opt_level = get_shaderc_optimization_level(options.optimization);
    shaderc_options.SetOptimizationLevel(opt_level);

    auto shader_name = name ? name : "unknown";
    auto result = compiler.CompileGlslToSpv(
        glsl, get_shaderc_kind(default_kind), shader_name, shaderc_options);

    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        ARS_LOG_ERROR("Failed to compile GLSL shader {}: {}",
                      shader_name,
                      result.GetErrorMessage());
        return {};
    }
    auto beg = reinterpret_cast<const char *>(result.begin());
    auto end = reinterpret_cast<const char *>(result.end());
    return {beg, end};
}

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
    IContext *context, MaterialPropertyBlockInfo info)
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

IContext *MaterialPropertyBlockLayout::context() const {
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

std::vector<std::shared_ptr<ITexture>>
MaterialPropertyBlock::referenced_textures() {
    std::vector<std::shared_ptr<ITexture>> textures{};
    auto props = _layout->info().properties;
    for (int i = 0; i < props.size(); i++) {
        if (props[i].type == MaterialPropertyType::Texture) {
            textures.push_back(get_texture_by_index(i));
        }
    }
    return textures;
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
} // namespace ars::render