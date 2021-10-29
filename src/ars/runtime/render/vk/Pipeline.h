#pragma once

#include "Vulkan.h"
#include <array>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace ars::render::vk {
class Context;
class RenderPass;
class Texture;

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

struct ShaderLocalSize {
    uint32_t x = 1;
    uint32_t y = 1;
    uint32_t z = 1;

    // Utility method for dispatch. Will round up thread count to multiples of
    // local size.
    void dispatch(CommandBuffer *cmd,
                  uint32_t thread_x,
                  uint32_t thread_y,
                  uint32_t thread_z) const;
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

    [[nodiscard]] ShaderLocalSize local_size() const;

  private:
    void load_reflection_info(size_t code_size, const uint8_t *code);

    Context *_context = nullptr;
    std::string _entry{};
    ShaderLocalSize _local_size{};
    PipelineLayoutInfo _layout_info{};
    VkShaderStageFlagBits _stage{};
    VkShaderModule _module = VK_NULL_HANDLE;
};

class Pipeline {
  public:
    Pipeline(Context *context, VkPipelineBindPoint bind_point);

    ARS_NO_COPY_MOVE(Pipeline);

    ~Pipeline();

    [[nodiscard]] VkPipeline pipeline() const;
    [[nodiscard]] VkPipelineLayout pipeline_layout() const;
    [[nodiscard]] VkDescriptorSet alloc_desc_set(uint32_t set) const;
    [[nodiscard]] Context *context() const;
    [[nodiscard]] VkPipelineBindPoint bind_point() const;

  protected:
    void init_layout(const PipelineLayoutInfo &pipeline_layout_info,
                     uint32_t push_constant_range_count,
                     VkPushConstantRange *push_constant_ranges);

    Context *_context = nullptr;

    std::array<VkDescriptorSetLayout, MAX_DESC_SET_COUNT> _descriptor_layouts{};
    PipelineLayoutInfo _pipeline_layout_info{};
    VkPipelineLayout _pipeline_layout = VK_NULL_HANDLE;
    VkPipeline _pipeline = VK_NULL_HANDLE;
    VkPipelineBindPoint _bind_point = VK_PIPELINE_BIND_POINT_MAX_ENUM;
};

// Use reversed-Z by default
VkPipelineDepthStencilStateCreateInfo
enabled_depth_stencil_state(bool depth_write = true);
VkPipelineColorBlendAttachmentState
create_attachment_blend_state(VkBlendFactor src_factor,
                              VkBlendFactor dst_factor);

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

class GraphicsPipeline : public Pipeline {
  public:
    GraphicsPipeline(Context *context, const GraphicsPipelineInfo &info);

  private:
    void init_layout(const GraphicsPipelineInfo &info);
    void init_pipeline(const GraphicsPipelineInfo &info);
};

struct ComputePipelineInfo {
    Shader *shader = nullptr;

    // Push constant ranges can not be inferred from shader reflection data as
    // offsets are unknown.
    uint32_t push_constant_range_count = 0;
    VkPushConstantRange *push_constant_ranges = nullptr;
};

class ComputePipeline : public Pipeline {
  public:
    ComputePipeline(Context *context, const ComputePipelineInfo &info);

    [[nodiscard]] ShaderLocalSize local_size() const;

  private:
    void init_layout(const ComputePipelineInfo &info);
    void init_pipeline(const ComputePipelineInfo &info);

    ShaderLocalSize _local_size{};
};

// Utility struct for descriptor binding.
//
// Call commit() after setup descriptor bindings to update and bind descriptors
// to command buffer.
//
// This struct holds shallow reference to data, data should
// be alive on commit.
struct DescriptorEncoder {
  public:
    DescriptorEncoder();

    void set_combined_image_sampler(uint32_t set,
                                    uint32_t binding,
                                    Texture *texture);

    void set_storage_image(uint32_t set, uint32_t binding, Texture *texture);

    void set_uniform_buffer(uint32_t set,
                            uint32_t binding,
                            void *data,
                            size_t data_size);

    template <typename T>
    void set_uniform_buffer(uint32_t set, uint32_t binding, const T &data) {
        static_assert(std::is_pod_v<T>);
        set_uniform_buffer(set, binding, (void *)&data, sizeof(data));
    }

    void commit(CommandBuffer *cmd, Pipeline *pipeline);

  private:
    struct CombinedImageSamplerData {
        Texture *texture = nullptr;
    };

    struct StorageImageData {
        Texture *texture = nullptr;
    };

    struct UniformBufferData {
        void *data = nullptr;
        size_t size{};
    };

    using BindingData = std::
        variant<CombinedImageSamplerData, StorageImageData, UniformBufferData>;

    struct BindingInfo {
        uint32_t set{};
        uint32_t binding{};
        BindingData data{};
    };

    void set_binding(uint32_t set, uint32_t binding, const BindingData &data);

    int _binding_indices[MAX_DESC_SET_COUNT][MAX_DESC_BINDING_COUNT]{};
    std::vector<BindingInfo> _bindings;
};
} // namespace ars::render::vk