#pragma once

#include "Vulkan.h"
#include <array>
#include <optional>
#include <string>
#include <vector>

namespace ars::render::vk {
class Context;

struct DescriptorSetInfo {
    std::array<std::optional<VkDescriptorSetLayoutBinding>, 16> bindings{};

    VkDescriptorSetLayout create_desc_set_layout(Device *device) const;
};

struct PipelineLayoutInfo {
    std::array<std::optional<DescriptorSetInfo>, 16> sets{};
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

struct GraphicsPipelineInfo {
    std::vector<Shader *> shaders{};
    VkRenderPass render_pass = VK_NULL_HANDLE;
    uint32_t subpass = 0;
};

class GraphicsPipeline {
  public:
    GraphicsPipeline(Context *context, const GraphicsPipelineInfo &info);

    ARS_NO_COPY_MOVE(GraphicsPipeline);

    ~GraphicsPipeline();

    [[nodiscard]] VkPipeline pipeline() const;

  private:
    void init_layout(const GraphicsPipelineInfo &info);
    void init_pipeline(const GraphicsPipelineInfo &info);

    Context *_context = nullptr;
    std::vector<VkDescriptorSetLayout> _descriptor_layouts{};
    VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;
    VkPipeline _pipeline = VK_NULL_HANDLE;
};
} // namespace ars::render::vk