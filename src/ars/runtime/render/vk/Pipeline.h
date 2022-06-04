#pragma once

#include "Buffer.h"
#include "Vulkan.h"
#include <array>
#include <ars/runtime/core/misc/Span.h>
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

    [[nodiscard]] std::optional<VkDescriptorSetLayoutBinding>
    binding(uint32_t set, uint32_t binding) const;
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

    void dispatch(CommandBuffer *cmd, const VkExtent3D &thread_extent) const;
};

class Shader {
  public:
    static std::unique_ptr<Shader>
    find_precompiled(Context *context,
                     const char *name,
                     const std::vector<const char *> &flags = {});

    static std::unique_ptr<Shader> from_spirv(Context *context,
                                              const char *name,
                                              const uint8_t *spirv_code,
                                              size_t code_size);

    static std::unique_ptr<Shader> from_spirv(Context *context,
                                              const char *name,
                                              const std::vector<uint8_t> &code);

    Shader(Context *context,
           const char *name,
           const uint8_t *spirv_code,
           size_t code_size);

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
    [[nodiscard]] const PipelineLayoutInfo &pipeline_layout_info() const;
    [[nodiscard]] VkDescriptorSet alloc_desc_set(uint32_t set) const;
    [[nodiscard]] Context *context() const;
    [[nodiscard]] VkPipelineBindPoint bind_point() const;

    void bind(CommandBuffer *cmd) const;

  protected:
    void init_layout(const PipelineLayoutInfo &pipeline_layout_info,
                     uint32_t push_constant_range_count,
                     const VkPushConstantRange *push_constant_ranges);
    void set_name(const std::string &name);

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

struct SubpassInfo {
    RenderPass *render_pass = nullptr;
    uint32_t index = 0;

    [[nodiscard]] VkSubpassDescription description() const;
};

struct GraphicsPipelineInfo {
    std::optional<std::string> name;
    std::vector<Shader *> shaders{};
    SubpassInfo subpass{};

    // Push constant ranges can not be inferred from shader reflection data as
    // offsets are unknown.
    uint32_t push_constant_range_count = 0;
    const VkPushConstantRange *push_constant_ranges = nullptr;

    const VkPipelineVertexInputStateCreateInfo *vertex_input = nullptr;
    const VkPipelineColorBlendStateCreateInfo *blend = nullptr;
    const VkPipelineDepthStencilStateCreateInfo *depth_stencil = nullptr;
    const VkPipelineRasterizationStateCreateInfo *raster = nullptr;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    const VkSpecializationInfo *specialization_info = nullptr;
};

class GraphicsPipeline : public Pipeline {
  public:
    GraphicsPipeline(Context *context, const GraphicsPipelineInfo &info);

  private:
    void init_layout(const GraphicsPipelineInfo &info);
    void init_pipeline(const GraphicsPipelineInfo &info);
};

struct ComputePipelineInfo {
    std::optional<std::string> name;
    Shader *shader = nullptr;

    // Push constant ranges can not be inferred from shader reflection data as
    // offsets are unknown.
    uint32_t push_constant_range_count = 0;
    const VkPushConstantRange *push_constant_ranges = nullptr;
    const VkSpecializationInfo *specialization_info = nullptr;
};

class ComputePipeline : public Pipeline {
  public:
    ComputePipeline(Context *context, const ComputePipelineInfo &info);

    static std::unique_ptr<ComputePipeline>
    create(Context *context,
           const char *shader_name,
           const std::vector<const char *> &flags = {});

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

    // If level is given, only the specified level of texture will be bound
    void set_texture(uint32_t set,
                     uint32_t binding,
                     Texture *texture,
                     std::optional<uint32_t> level = std::nullopt);

    // This method will copy content of underlying data.
    // This method used to do no copy and expect the caller to keep the
    // ownership of the data, which is proven to be too error-prone in practice.
    void set_buffer_data(uint32_t set,
                         uint32_t binding,
                         void *data,
                         size_t data_size);

    template <typename T>
    void set_buffer_data(uint32_t set, uint32_t binding, const T &data) {
        using DataView = details::BufferDataViewTrait<T>;
        set_buffer_data(
            set, binding, DataView::ptr(data), DataView::size(data));
    }

    void set_buffer(uint32_t set,
                    uint32_t binding,
                    Buffer *buffer,
                    VkDeviceSize offset,
                    VkDeviceSize size);

    void set_buffer(uint32_t set, uint32_t binding, Buffer *buffer);

    // Does not take the ownership of textures
    void set_textures(uint32_t set,
                      uint32_t binding,
                      ars::Span<Handle<Texture>> textures);

    void commit(CommandBuffer *cmd, Pipeline *pipeline);

  private:
    struct ImageInfo {
        Texture *texture = nullptr;
        std::optional<uint32_t> level{};
    };

    struct BufferDataInfo {
        std::unique_ptr<uint8_t[]> data{};
        size_t size{};
    };

    struct BufferInfo {
        Buffer *buffer = nullptr;
        VkDeviceSize offset{};
        VkDeviceSize size{};
    };

    using BindingData =
        std::variant<std::vector<ImageInfo>, BufferDataInfo, BufferInfo>;

    struct BindingInfo {
        uint32_t set{};
        uint32_t binding{};
        BindingData data{};
    };

    void set_binding(uint32_t set, uint32_t binding, BindingData data);

    int _binding_indices[MAX_DESC_SET_COUNT][MAX_DESC_BINDING_COUNT]{};
    std::vector<BindingInfo> _bindings;
};

} // namespace ars::render::vk