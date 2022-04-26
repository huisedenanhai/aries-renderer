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
    return ars::visit(
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

bool MaterialInfo::operator<(const MaterialInfo &rhs) const {
    auto sm = static_cast<uint32_t>(shading_model);
    auto rhs_sm = static_cast<uint32_t>(rhs.shading_model);
    if (sm < rhs_sm) {
        return true;
    }
    if (sm > rhs_sm) {
        return false;
    }
    return features < rhs.features;
}
} // namespace ars::render