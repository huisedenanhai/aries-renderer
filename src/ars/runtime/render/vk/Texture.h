#pragma once

#include "../ITexture.h"
#include "Vulkan.h"
#include <vector>

namespace ars::render::vk {
class Context;
struct RenderGraph;

struct TextureCreateInfo {
    VkImageType image_type{};
    VkImageViewType view_type{};
    VkFormat format{};
    VkExtent3D extent{};
    // mip_level will be clamped in range when create texture.
    uint32_t mip_levels{};
    uint32_t array_layers{};
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    VkImageUsageFlags usage{};
    VkImageAspectFlags aspect_mask{};

    VkFilter min_filter = VK_FILTER_LINEAR;
    VkFilter mag_filter = VK_FILTER_LINEAR;
    VkSamplerMipmapMode mipmap_mode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    VkSamplerAddressMode address_mode_u = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_v = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    VkSamplerAddressMode address_mode_w = VK_SAMPLER_ADDRESS_MODE_REPEAT;

    static TextureCreateInfo sampled(VkImageType image_type,
                                     VkImageViewType view_type,
                                     VkFormat format,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t depth,
                                     uint32_t mip_levels,
                                     VkSamplerAddressMode address_mode);

    static TextureCreateInfo sampled_2d(
        VkFormat format,
        uint32_t width,
        uint32_t height,
        uint32_t mip_levels = MAX_MIP_LEVELS,
        VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT);

    static TextureCreateInfo sampled_3d(
        VkFormat format,
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        uint32_t mip_levels = MAX_MIP_LEVELS,
        VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT);
};

VkFormat translate(render::Format format);

TextureCreateInfo translate(const render::TextureInfo &info);

class Texture : public ITextureHandle {
  public:
    Texture(Context *context, const TextureCreateInfo &info);

    ARS_NO_COPY_MOVE(Texture);

    ~Texture() override;

    void set_data(void *data,
                  size_t size,
                  uint32_t mip_level,
                  uint32_t layer,
                  int32_t x_offset,
                  int32_t y_offset,
                  int32_t z_offset,
                  uint32_t x_size,
                  uint32_t y_size,
                  uint32_t z_size);

    void generate_mipmap(CommandBuffer *cmd,
                         VkPipelineStageFlags src_stage_mask,
                         VkAccessFlags src_access_mask,
                         VkPipelineStageFlags dst_stage_mask,
                         VkAccessFlags dst_access_mask);

    static void generate_mipmap(const Handle<Texture> &tex, RenderGraph &rg);

    [[nodiscard]] Context *context() const;

    [[nodiscard]] VkSampler sampler() const;
    [[nodiscard]] VkImageLayout layout() const;
    [[nodiscard]] VkImageView image_view() const;
    [[nodiscard]] VkImage image() const;

    [[nodiscard]] VkImageView image_view_of_level(uint32_t level);

    [[nodiscard]] const TextureCreateInfo &info() const;

    void assure_layout(VkImageLayout layout);

    [[nodiscard]] VkImageSubresourceRange subresource_range() const;
    [[nodiscard]] VkImageSubresourceRange
    subresource_range(uint32_t base_mip_level, uint32_t level_count) const;

    // Transfer all subresources to the target layout and set the _layout field.
    // This method assumes all layers and levels of the image are in the same
    // layout, and public methods should maintain this invariance.
    //
    // This method inserts a barrier to the command buffer no matter whether the
    // image is in the target layout.
    void transfer_layout(CommandBuffer *cmd,
                         VkImageLayout dst_layout,
                         VkShaderStageFlags src_stage_mask,
                         VkAccessFlags src_access_mask,
                         VkShaderStageFlags dst_stage_mask,
                         VkAccessFlags dst_access_mask);

  private:
    void init();

    // Call this method after _image and _info initialized
    [[nodiscard]] VkImageView create_image_view(uint32_t base_mip_level,
                                                uint32_t mip_level_count);

    Context *_context = nullptr;
    VmaAllocation _allocation = VK_NULL_HANDLE;
    VkImage _image = VK_NULL_HANDLE;
    VkImageView _image_view = VK_NULL_HANDLE;
    std::vector<VkImageView> _image_view_of_levels{};
    VkSampler _sampler = VK_NULL_HANDLE;
    // Public methods should ensure all layers and levels of the image are in
    // the same layout
    VkImageLayout _layout = VK_IMAGE_LAYOUT_UNDEFINED;

    TextureCreateInfo _info{};
};

class TextureAdapter : public ITexture {
  public:
    TextureAdapter(const TextureInfo &info, Handle<Texture> texture);

    ARS_DEFAULT_COPY_MOVE(TextureAdapter);

    void set_data(void *data,
                  size_t size,
                  uint32_t mip_level,
                  uint32_t layer,
                  int32_t x_offset,
                  int32_t y_offset,
                  int32_t z_offset,
                  uint32_t x_size,
                  uint32_t y_size,
                  uint32_t z_size) override;

    void generate_mipmap() override;

    [[nodiscard]] Handle<Texture> texture() const;
    ITextureHandle *handle() override;

  private:
    Handle<Texture> _texture{};
};

Handle<Texture> upcast(ITexture *texture);
Texture *upcast(ITextureHandle *handle);
} // namespace ars::render::vk