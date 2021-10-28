#include "Pipeline.h"
#include "Context.h"
#include "Descriptor.h"
#include <ars/runtime/core/Log.h>
#include <cassert>
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
        ARS_LOG_CRITICAL("Failed to load shader module ", name);
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

    VkDescriptorSetLayoutBinding valid_bindings[MAX_DESC_BINDING_COUNT]{};
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
        ARS_LOG_CRITICAL("Failed to create per frame descriptor layout");
    }
    return layout;
}

namespace {
template <typename T>
std::optional<T> merge(const std::optional<T> &lhs,
                       const std::optional<T> &rhs);

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

} // namespace

GraphicsPipeline::GraphicsPipeline(Context *context,
                                   const GraphicsPipelineInfo &info)
    : Pipeline(context) {
    init_layout(info);
    init_pipeline(info);
}

Pipeline::~Pipeline() {
    auto device = _context->device();

    for (auto desc_layout : _descriptor_layouts) {
        if (desc_layout != VK_NULL_HANDLE) {
            device->Destroy(desc_layout);
        }
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

    Pipeline::init_layout(pipeline_layout_info,
                          info.push_constant_range_count,
                          info.push_constant_ranges);
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
    if (info.vertex_input != nullptr) {
        create_info.pVertexInputState = info.vertex_input;
    }

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

    VkPipelineDepthStencilStateCreateInfo depth_stencil{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    create_info.pDepthStencilState = &depth_stencil;
    if (info.depth_stencil != nullptr) {
        create_info.pDepthStencilState = info.depth_stencil;
    }

    assert(info.render_pass != nullptr);
    auto &subpass = info.render_pass->subpasses()[info.subpass];

    std::vector<VkPipelineColorBlendAttachmentState> attachments{};
    attachments.resize(subpass.colorAttachmentCount);
    for (auto &attach : attachments) {
        attach.colorWriteMask = 0xF; // RGBA
    }

    VkPipelineColorBlendStateCreateInfo blend{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    blend.attachmentCount = static_cast<uint32_t>(attachments.size());
    blend.pAttachments = attachments.data();

    create_info.pColorBlendState = &blend;
    if (info.blend != nullptr) {
        create_info.pColorBlendState = info.blend;
    }

    VkDynamicState dyn_states[] = {VK_DYNAMIC_STATE_VIEWPORT,
                                   VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamic_state{
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
    dynamic_state.dynamicStateCount = std::size(dyn_states);
    dynamic_state.pDynamicStates = dyn_states;

    create_info.pDynamicState = &dynamic_state;

    create_info.layout = _pipeline_layout;
    create_info.renderPass = info.render_pass->render_pass();
    create_info.subpass = info.subpass;

    if (_context->device()->Create(
            _context->pipeline_cache(), 1, &create_info, &_pipeline) !=
        VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create graphics pipeline");
    }
}

VkPipeline Pipeline::pipeline() const {
    return _pipeline;
}

VkDescriptorSet Pipeline::alloc_desc_set(uint32_t set) const {
    return _context->descriptor_arena()->alloc(
        _descriptor_layouts[set], _pipeline_layout_info.sets[set].value());
}

VkPipelineLayout Pipeline::pipeline_layout() const {
    return _pipeline_layout;
}

Pipeline::Pipeline(Context *context) : _context(context) {}

void Pipeline::init_layout(const PipelineLayoutInfo &pipeline_layout_info,
                           uint32_t push_constant_range_count,
                           VkPushConstantRange *push_constant_ranges) {
    _pipeline_layout_info = pipeline_layout_info;

    std::vector<VkDescriptorSetLayout> valid_desc_sets{};
    valid_desc_sets.reserve(MAX_DESC_SET_COUNT);

    for (int i = 0; i < MAX_DESC_SET_COUNT; i++) {
        auto &s = pipeline_layout_info.sets[i];
        if (s.has_value()) {
            auto desc_set = s->create_desc_set_layout(_context->device());
            _descriptor_layouts[i] = desc_set;
            valid_desc_sets.push_back(desc_set);
        } else {
            _descriptor_layouts[i] = VK_NULL_HANDLE;
        }
    }

    VkPipelineLayoutCreateInfo create_info{
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};

    create_info.setLayoutCount = static_cast<uint32_t>(valid_desc_sets.size());
    create_info.pSetLayouts = valid_desc_sets.data();
    create_info.pushConstantRangeCount = push_constant_range_count;
    create_info.pPushConstantRanges = push_constant_ranges;

    if (_context->device()->Create(&create_info, &_pipeline_layout) !=
        VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create pipeline layout");
    }
}

VkPipelineColorBlendAttachmentState
create_attachment_blend_state(VkBlendFactor src_factor,
                              VkBlendFactor dst_factor) {
    VkPipelineColorBlendAttachmentState blend{};

    blend.blendEnable = VK_TRUE;
    blend.colorWriteMask = 0xF; // RGBA
    blend.colorBlendOp = VK_BLEND_OP_ADD;
    blend.alphaBlendOp = VK_BLEND_OP_ADD;
    blend.srcColorBlendFactor = src_factor;
    blend.srcAlphaBlendFactor = src_factor;
    blend.dstColorBlendFactor = dst_factor;
    blend.dstAlphaBlendFactor = dst_factor;

    return blend;
}

VkPipelineDepthStencilStateCreateInfo
enabled_depth_stencil_state(bool depth_write) {
    VkPipelineDepthStencilStateCreateInfo info{
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
    info.depthTestEnable = VK_TRUE;
    info.depthWriteEnable = depth_write ? VK_TRUE : VK_FALSE;
    info.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;

    return info;
}

ComputePipeline::ComputePipeline(Context *context,
                                 const ComputePipelineInfo &info)
    : Pipeline(context) {
    assert(info.shader);
    init_layout(info);
    init_pipeline(info);
}

void ComputePipeline::init_layout(const ComputePipelineInfo &info) {
    Pipeline::init_layout(info.shader->pipeline_layout_info(),
                          info.push_constant_range_count,
                          info.push_constant_ranges);
}

void ComputePipeline::init_pipeline(const ComputePipelineInfo &info) {
    VkComputePipelineCreateInfo create_info{
        VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO};

    auto shader = info.shader;
    auto &si = create_info.stage;
    si.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    si.module = shader->module();
    si.stage = shader->stage();
    si.pName = shader->entry();

    create_info.layout = _pipeline_layout;

    if (_context->device()->Create(
            _context->pipeline_cache(), 1, &create_info, &_pipeline) !=
        VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create compute pipeline");
    }
}
} // namespace ars::render::vk
