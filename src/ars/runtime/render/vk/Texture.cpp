#include "Texture.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>
#include <cassert>

namespace ars::render::vk {
namespace {
constexpr VkImageUsageFlags IMAGE_USAGE_SAMPLED_COLOR =
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
}

Texture::Texture(Context *context, const TextureCreateInfo &info)
    : _context(context), _info(info) {
    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.arrayLayers = info.array_layers;
    image_info.imageType = info.image_type;
    image_info.mipLevels = info.mip_levels;
    image_info.samples = info.samples;
    image_info.extent = info.extent;
    image_info.format = info.format;
    image_info.usage = info.usage;

    image_info.initialLayout = _layout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    if (info.view_type == VK_IMAGE_VIEW_TYPE_CUBE ||
        info.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
        image_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    // All textures are used exclusively.
    // Concurrent access disables some potential optimizations from driver.
    // (Like Delta Color Compression(DCC) from AMD drivers, see
    // https://www.khronos.org/assets/uploads/developers/presentations/06-Optimising-aaa-vulkan-title-on-desktop-May19.pdf)
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    auto vma = context->vma();
    if (vmaCreateImage(vma->raw(),
                       &image_info,
                       &alloc_info,
                       &_image,
                       &_allocation,
                       nullptr) != VK_SUCCESS) {
        panic("Failed to create image");
    }

    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.image = _image;
    view_info.format = info.format;
    view_info.viewType = info.view_type;
    view_info.subresourceRange = get_subresource_range();
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    if (context->device()->Create(&view_info, &_image_view) != VK_SUCCESS) {
        panic("Failed to create image view");
    }
}

Texture::~Texture() {
    if (_image != VK_NULL_HANDLE) {
        vmaDestroyImage(_context->vma()->raw(), _image, _allocation);
    }
    if (_image_view != VK_NULL_HANDLE) {
        _context->device()->Destroy(_image_view);
    }
}

VkImageSubresourceRange Texture::get_subresource_range() const {
    VkImageSubresourceRange range{};
    range.aspectMask = _info.aspect_mask;
    range.baseArrayLayer = 0;
    range.layerCount = _info.array_layers;
    range.baseMipLevel = 0;
    range.levelCount = _info.mip_levels;

    return range;
}

void Texture::set_data(void *data,
                       size_t size,
                       uint32_t mip_level,
                       uint32_t layer,
                       int32_t x_offset,
                       int32_t y_offset,
                       int32_t z_offset,
                       uint32_t x_size,
                       uint32_t y_size,
                       uint32_t z_size) {
    _context->queue()->submit_once([&](CommandBuffer *cmd) {
        auto stage_buffer =
            _context->create_buffer(size,
                                    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                    VMA_MEMORY_USAGE_CPU_TO_GPU);
        stage_buffer->map_once(
            [&](void *ptr) { std::memcpy(ptr, data, size); });

        transfer_layout(cmd,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        0,
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_ACCESS_TRANSFER_WRITE_BIT);

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = mip_level;
        region.imageSubresource.baseArrayLayer = layer;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {x_offset, y_offset, z_offset};
        region.imageExtent = {x_size, y_size, z_size};

        cmd->CopyBufferToImage(
            stage_buffer->buffer(), _image, _layout, 1, &region);

        transfer_layout(cmd,
                        VK_IMAGE_LAYOUT_GENERAL,
                        VK_PIPELINE_STAGE_TRANSFER_BIT,
                        VK_ACCESS_TRANSFER_WRITE_BIT,
                        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                        0);
    });
}

void Texture::generate_mipmap() {
    VkFormatProperties format_properties;
    _context->instance()->GetPhysicalDeviceFormatProperties(
        _context->device()->physical_device(),
        _info.format,
        &format_properties);

    if (!(format_properties.optimalTilingFeatures &
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        log_error("Texture image format does not support linear blit, skip "
                  "mipmap generation");
        return;
    }

    if (_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
        log_error("Texture is not initialized, skip mipmap generation");
        return;
    }

    if (_info.mip_levels <= 1) {
        // Nothing to do
        return;
    }

    auto get_barrier = [&](int mip_level,
                           uint32_t level_count,
                           VkImageLayout old_layout,
                           VkImageLayout new_layout,
                           VkAccessFlags src_access_mask,
                           VkAccessFlags dst_access_mask) {
        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};

        barrier.subresourceRange = get_subresource_range();
        barrier.subresourceRange.baseMipLevel = mip_level;
        barrier.subresourceRange.levelCount = level_count;
        barrier.image = _image;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.srcAccessMask = src_access_mask;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstAccessMask = dst_access_mask;

        return barrier;
    };

    _context->queue()->submit_once([&](CommandBuffer *cmd) {
        auto mip_levels = static_cast<int>(_info.mip_levels);
        assert(mip_levels > 1);
        // transfer all levels to TRANSFER_DST_OPTIMAL
        {
            auto barrier = get_barrier(0,
                                       mip_levels,
                                       _layout,
                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                       0,
                                       VK_ACCESS_TRANSFER_WRITE_BIT);
            cmd->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 VK_PIPELINE_STAGE_TRANSFER_BIT,
                                 0,
                                 0,
                                 nullptr,
                                 0,
                                 nullptr,
                                 1,
                                 &barrier);
        }

        auto mip_width = static_cast<int32_t>(_info.extent.width);
        auto mip_height = static_cast<int32_t>(_info.extent.height);
        auto mip_depth = static_cast<int32_t>(_info.extent.depth);
        for (int i = 1; i < mip_levels; i++) {
            auto calc_next_mip_size = [](int32_t size) {
                return size > 1 ? size / 2 : 1;
            };
            auto next_mip_width = calc_next_mip_size(mip_width);
            auto next_mip_height = calc_next_mip_size(mip_height);
            auto next_mip_depth = calc_next_mip_size(mip_depth);

            // Transfer src level to TRANSFER_SRC_OPTIMAL
            {
                auto barrier = get_barrier(i - 1,
                                           1,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                           VK_ACCESS_TRANSFER_WRITE_BIT,
                                           VK_ACCESS_TRANSFER_READ_BIT);
                cmd->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     0,
                                     0,
                                     nullptr,
                                     0,
                                     nullptr,
                                     1,
                                     &barrier);
            }

            VkImageSubresourceLayers res{};
            res.aspectMask = get_subresource_range().aspectMask;
            res.baseArrayLayer = 0;
            res.layerCount = _info.array_layers;

            VkImageBlit region{};
            region.srcSubresource = res;
            region.srcSubresource.mipLevel = i - 1;
            region.srcOffsets[0] = {0, 0, 0};
            region.srcOffsets[1] = {mip_width, mip_height, mip_depth};
            region.dstSubresource = res;
            region.dstSubresource.mipLevel = i;
            region.dstOffsets[0] = {0, 0, 0};
            region.dstOffsets[1] = {
                next_mip_width, next_mip_height, next_mip_depth};

            cmd->BlitImage(_image,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                           _image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                           1,
                           &region,
                           VK_FILTER_LINEAR);

            mip_width = next_mip_width;
            mip_height = next_mip_height;
            mip_depth = next_mip_depth;
        }
        // Now all except the last layer are in the TRANSFER_SRC_OPTIMAL layout.
        // The last layer is in TRANSFER_DST_OPTIMAL layout.
        // Restore the original layout with two barriers
        VkImageMemoryBarrier barriers[2] = {
            get_barrier(0,
                        mip_levels - 1,
                        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        _layout,
                        VK_ACCESS_TRANSFER_READ_BIT,
                        0),
            get_barrier(mip_levels - 1,
                        1,
                        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        _layout,
                        VK_ACCESS_TRANSFER_WRITE_BIT,
                        0)};
        cmd->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                             VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             2,
                             barriers);
    });
}

void Texture::transfer_layout(CommandBuffer *cmd,
                              VkImageLayout dst_layout,
                              VkShaderStageFlags src_stage_mask,
                              VkAccessFlags src_access_mask,
                              VkShaderStageFlags dst_stage_mask,
                              VkAccessFlags dst_access_mask) {
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};

    auto queue = _context->queue();
    barrier.subresourceRange = get_subresource_range();
    barrier.image = _image;
    barrier.oldLayout = _layout;
    barrier.newLayout = dst_layout;
    barrier.srcQueueFamilyIndex = queue->family_index();
    barrier.srcAccessMask = src_access_mask;
    barrier.dstQueueFamilyIndex = queue->family_index();
    barrier.dstAccessMask = dst_access_mask;

    cmd->PipelineBarrier(
        src_stage_mask, dst_stage_mask, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    _layout = dst_layout;
}

TextureCreateInfo TextureCreateInfo::sampled_2d(VkFormat format,
                                                uint32_t width,
                                                uint32_t height,
                                                uint32_t mip_levels) {
    TextureCreateInfo info{};
    info.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.view_type = VK_IMAGE_VIEW_TYPE_2D;
    info.image_type = VK_IMAGE_TYPE_2D;
    info.format = format;
    info.usage = IMAGE_USAGE_SAMPLED_COLOR;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.mip_levels = mip_levels;
    info.array_layers = 1;
    info.extent = VkExtent3D{width, height, 1};

    return info;
}

TextureAdapter::TextureAdapter(const TextureInfo &info, Handle<Texture> texture)
    : ITexture(info), _texture(std::move(texture)) {}

void TextureAdapter::set_data(void *data,
                              size_t size,
                              uint32_t mip_level,
                              uint32_t layer,
                              int32_t x_offset,
                              int32_t y_offset,
                              int32_t z_offset,
                              uint32_t x_size,
                              uint32_t y_size,
                              uint32_t z_size) {
    _texture->set_data(data,
                       size,
                       mip_level,
                       layer,
                       x_offset,
                       y_offset,
                       z_offset,
                       x_size,
                       y_size,
                       z_size);
}

void TextureAdapter::generate_mipmap() {
    _texture->generate_mipmap();
}

VkFormat translate(render::Format format) {
    switch (format) {
    case Format::R8G8B8A8Srgb:
        return VK_FORMAT_R8G8B8A8_SRGB;
    }
    // should not happen
    return VK_FORMAT_UNDEFINED;
}

TextureCreateInfo translate(const TextureInfo &info) {
    TextureCreateInfo up{};
    up.format = translate(info.format);
    switch (info.type) {
    case TextureType::Texture2D:
        up.image_type = VK_IMAGE_TYPE_2D;
        up.view_type = VK_IMAGE_VIEW_TYPE_2D;
        break;
    }
    up.array_layers = info.array_layers;
    up.mip_levels = info.mip_levels;
    up.extent.width = info.width;
    up.extent.height = info.height;
    up.extent.depth = info.depth;
    up.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    up.samples = VK_SAMPLE_COUNT_1_BIT;
    up.usage = IMAGE_USAGE_SAMPLED_COLOR;

    return up;
}
} // namespace ars::render::vk
