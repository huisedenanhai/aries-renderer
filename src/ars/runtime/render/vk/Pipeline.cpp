#include "Pipeline.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>
#include <sstream>
#include <vulkan/spirv_reflect.h>

namespace ars::render::vk {
Shader::Shader(Context *context, const char *name) : _context(context) {
    auto code = load_spirv_code(name, nullptr, 0);

    load_reflection_info(code.size, code.data);

    VkShaderModuleCreateInfo info{VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
    info.pCode = reinterpret_cast<const uint32_t *>(code.data);
    info.codeSize = code.size;

    if (_context->device()->Create(&info, &_module) != VK_SUCCESS) {
        std::stringstream ss;
        ss << "Failed to load shader module " << name;
        panic(ss.str());
    }
}

Shader::~Shader() {
    if (_module != VK_NULL_HANDLE) {
        _context->device()->Destroy(_module);
    }
}

VkShaderModule Shader::module() const {
    return _module;
}

const PipelineLayoutInfo &Shader::pipeline_layout_info() const {
    return _layout_info;
}

void Shader::load_reflection_info(size_t code_size, const uint8_t *code) {
    spv_reflect::ShaderModule reflect(code_size, code);
    _stage = static_cast<VkShaderStageFlagBits>(reflect.GetShaderStage());
    _entry = reflect.GetEntryPointName();

    uint32_t set_count;
    std::vector<SpvReflectDescriptorSet *> sets;
    reflect.EnumerateDescriptorSets(&set_count, nullptr);
    sets.resize(set_count);
    reflect.EnumerateDescriptorSets(&set_count, sets.data());

    for (uint32_t i = 0; i < set_count; i++) {
        auto &desc_set = sets[i];
        DescriptorSetInfo set_info{};

        for (uint32_t b = 0; b < desc_set->binding_count; b++) {
            VkDescriptorSetLayoutBinding binding_info{};
            auto binding = desc_set->bindings[b];

            binding_info.binding = binding->binding;
            binding_info.stageFlags = _stage;
            binding_info.descriptorCount = binding->count;
            binding_info.descriptorType =
                static_cast<VkDescriptorType>(binding->descriptor_type);

            set_info.bindings[binding->binding] = binding_info;
        }

        _layout_info.sets[desc_set->set] = set_info;
    }
}

VkShaderStageFlagBits Shader::stage() const {
    return _stage;
}

const char *Shader::entry() const {
    return _entry.c_str();
}

VkDescriptorSetLayout
DescriptorSetInfo::create_desc_set_layout(Device *device) const {
    VkDescriptorSetLayoutCreateInfo info{
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};

    VkDescriptorSetLayoutBinding valid_bindings[16]{};
    uint32_t binding_count = 0;
    for (auto &b : bindings) {
        if (b.has_value()) {
            valid_bindings[binding_count++] = b.value();
        }
    }

    info.bindingCount = binding_count;
    info.pBindings = valid_bindings;

    VkDescriptorSetLayout layout;
    if (device->Create(&info, &layout) != VK_SUCCESS) {
        panic("failed to create per frame descriptor layout");
    }
    return layout;
}

namespace {
template <typename T>
std::optional<T> merge(const std::optional<T> &lhs,
                       const std::optional<T> &rhs) {
    if (!lhs.has_value()) {
        return rhs;
    }

    if (rhs.has_value()) {
        return merge(lhs.value(), rhs.value());
    }

    return lhs;
}

VkDescriptorSetLayoutBinding merge(const VkDescriptorSetLayoutBinding &lhs,
                                   const VkDescriptorSetLayoutBinding &rhs) {
    auto result = lhs;
    result.stageFlags |= rhs.stageFlags;
    return result;
}

DescriptorSetInfo merge(const DescriptorSetInfo &lhs,
                        const DescriptorSetInfo &rhs) {
    DescriptorSetInfo result{};
    for (int i = 0; i < result.bindings.size(); i++) {
        result.bindings[i] = merge(lhs.bindings[i], rhs.bindings[i]);
    }
    return result;
}

PipelineLayoutInfo merge(const PipelineLayoutInfo &lhs,
                         const PipelineLayoutInfo &rhs) {
    PipelineLayoutInfo result{};
    for (int i = 0; i < result.sets.size(); i++) {
        result.sets[i] = merge(lhs.sets[i], rhs.sets[i]);
    }
    return result;
}
} // namespace

GraphicsPipeline::GraphicsPipeline(Context *context,
                                   const GraphicsPipelineInfo &info)
    : _context(context) {
    init_layout(info);
    init_pipeline(info);
}

GraphicsPipeline::~GraphicsPipeline() {
    auto device = _context->device();

    for (auto desc_layout : _descriptor_layouts) {
        device->Destroy(desc_layout);
    }

    if (_pipeline_layout != VK_NULL_HANDLE) {
        device->Destroy(_pipeline_layout);
    }

    if (_pipeline != VK_NULL_HANDLE) {
        device->Destroy(_pipeline);
    }
}

void GraphicsPipeline::init_layout(const GraphicsPipelineInfo &info) {
    PipelineLayoutInfo pipeline_layout_info{};
    for (auto shader : info.shaders) {
        pipeline_layout_info =
            merge(pipeline_layout_info, shader->pipeline_layout_info());
    }

    _descriptor_layouts.reserve(16);
    for (auto &s : pipeline_layout_info.sets) {
        if (!s.has_value()) {
            continue;
        }
        _descriptor_layouts.push_back(
            s->create_desc_set_layout(_context->device()));
    }

    VkPipelineLayoutCreateInfo create_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    create_info.setLayoutCount =
        static_cast<uint32_t>(_descriptor_layouts.size());
    create_info.pSetLayouts = _descriptor_layouts.data();

    if (_context->device()->Create(&create_info, &_pipeline_layout) !=
        VK_SUCCESS) {
        panic("Failed to create pipeline layout");
    }
}

void GraphicsPipeline::init_pipeline(const GraphicsPipelineInfo &info) {
    VkGraphicsPipelineCreateInfo create_info{
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

    std::vector<VkPipelineShaderStageCreateInfo> stages{};
    stages.reserve(info.shaders.size());
    for (auto shader : info.shaders) {
        VkPipelineShaderStageCreateInfo si{
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO};
        si.module = shader->module();
        si.stage = shader->stage();
        si.pName = shader->entry();

        stages.push_back(si);
    }

    create_info.stageCount = static_cast<uint32_t>(stages.size());
    create_info.pStages = stages.data();

    VkPipelineVertexInputStateCreateInfo vert_input{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    create_info.pVertexInputState = &vert_input;

    VkPipelineInputAssemblyStateCreateInfo input_assembly{
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    create_info.pInputAssemblyState = &input_assembly;

    create_info.pTessellationState = nullptr;

    VkPipelineViewportStateCreateInfo viewport_state{
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
    VkViewport viewport{0, 0, 1, 1, 0, 1};
    VkRect2D scissor{{0, 0}, {1, 1}};

    viewport_state.viewportCount = 1;
    viewport_state.pViewports = &viewport;
    viewport_state.scissorCount = 1;
    viewport_state.pScissors = &scissor;

    create_info.pViewportState = &viewport_state;

    VkPipelineRasterizationStateCreateInfo raster{
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
    raster.cullMode = VK_CULL_MODE_NONE;
    raster.polygonMode = VK_POLYGON_MODE_FILL;
    raster.lineWidth = 1.0f;

    create_info.pRasterizationState = &raster;

    VkPipelineMultisampleStateCreateInfo multisample{
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
    multisample.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    create_info.pMultisampleState = &multisample;

    create_info.pDepthStencilState = nullptr;

    VkPipelineColorBlendStateCreateInfo blend{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

    VkPipelineColorBlendAttachmentState attachment{};
    attachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    blend.attachmentCount = 1;
    blend.pAttachments = &attachment;

    create_info.pColorBlendState = &blend;

    VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                   VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic_state.dynamicStateCount = std::size(dyn_states);
    dynamic_state.pDynamicStates = dyn_states;

    create_info.pDynamicState = &dynamic_state;

    create_info.layout = _pipeline_layout;
    create_info.renderPass = info.render_pass;
    create_info.subpass = info.subpass;

    if (_context->device()->Create(
            _context->pipeline_cache(), 1, &create_info, &_pipeline) !=
        VK_SUCCESS) {
        panic("Failed to create pipeline for swapchain present");
    }
}

VkPipeline GraphicsPipeline::pipeline() const {
    return _pipeline;
}
} // namespace ars::render::vk
