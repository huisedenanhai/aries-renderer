#pragma once

#include "Vulkan.h"
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace ars::render::vk {
class Context;
class RenderPass;

constexpr uint32_t MAX_DESC_BINDING_COUNT = 16;
constexpr uint32_t MAX_DESC_SET_COUNT = 16;

struct DescriptorSetInfo {
    std::array<std::optional<VkDescriptorSetLayoutBinding>,
               MAX_DESC_BINDING_COUNT>
        bindings{};

    VkDescriptorSetLayout create_desc_set_layout(Device *device) const;
};

struct PipelineLayoutInfo {
    std::array<std::optional<DescriptorSetInfo>, MAX_DESC_SET_COUNT> sets{};
};

class Shader {
  public:
    Shader(Context *context, const char *name);

    ARS_NO_COPY_MOVE(Shader);

    ~Shader();

    [[nodiscard]] VkShaderModule module() const;

    [[nodiscard]] VkShaderStageFlagBits stage() const;

    [[nodiscard]] const char *entry() const;

    [[nodiscard]] const PipelineLayoutInfo &pipeline_layout_info() const;

  private:
    void load_reflection_info(size_t code_size, const uint8_t *code);

    Context *_context = nullptr;
    std::string _entry{};
    PipelineLayoutInfo _layout_info{};
    VkShaderStageFlagBits _stage{};
    VkShaderModule _module = VK_NULL_HANDLE;
};

// Use reversed-Z by default
VkPipelineDepthStencilStateCreateInfo
enabled_depth_stencil_state(bool depth_write = true);

struct GraphicsPipelineInfo {
    std::vector<Shader *> shaders{};
    RenderPass *render_pass = nullptr;
    uint32_t subpass = 0;

    // Push constant ranges can not be inferred from shader reflection data as
    // offsets are unknown.
    uint32_t push_constant_range_count = 0;
    VkPushConstantRange *push_constant_ranges = nullptr;

    VkPipelineVertexInputStateCreateInfo *vertex_input = nullptr;
    VkPipelineColorBlendStateCreateInfo *blend = nullptr;
    VkPipelineDepthStencilStateCreateInfo *depth_stencil = nullptr;
};

class GraphicsPipeline {
  public:
    GraphicsPipeline(Context *context, const GraphicsPipelineInfo &info);

    ARS_NO_COPY_MOVE(GraphicsPipeline);

    ~GraphicsPipeline();

    [[nodiscard]] VkPipeline pipeline() const;
    [[nodiscard]] VkPipelineLayout pipeline_layout() const;
    [[nodiscard]] VkDescriptorSet alloc_desc_set(uint32_t set) const;

  private:
    void init_layout(const GraphicsPipelineInfo &info);
    void init_pipeline(const GraphicsPipelineInfo &info);

    Context *_context = nullptr;

    std::array<VkDescriptorSetLayout, MAX_DESC_SET_COUNT> _descriptor_layouts{};
    PipelineLayoutInfo _pipeline_layout_info{};
    VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;
    VkPipeline _pipeline = VK_NULL_HANDLE;
};

VkPipelineColorBlendAttachmentState
create_attachment_blend_state(VkBlendFactor src_factor,
                              VkBlendFactor dst_factor);
} // namespace ars::render::vk