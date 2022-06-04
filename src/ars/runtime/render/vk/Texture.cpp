#include "Texture.h"
#include "Bindless.h"
#include "Context.h"
#include "Profiler.h"
#include "RenderGraph.h"
#include <ars/runtime/core/Log.h>
#include <cassert>

namespace ars::render::vk {
namespace {
constexpr VkImageUsageFlags IMAGE_USAGE_SAMPLED_COLOR =
    VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT |
    VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

VkImageUsageFlags
remove_unsupported_image_usages(VkImageUsageFlags usages,
                                VkFormatFeatureFlags features) {
    auto check = [&](VkImageUsageFlagBits usage_bit,
                     VkFormatFeatureFlagBits required_feature) {
        if (!(features & required_feature)) {
            usages &= (~usage_bit);
        }
    };
    check(VK_IMAGE_USAGE_SAMPLED_BIT, VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT);
    check(VK_IMAGE_USAGE_STORAGE_BIT, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT);
    check(VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
          VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT);
    check(VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
          VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);

    return usages;
}
} // namespace

Texture::Texture(Context *context, const TextureCreateInfo &info)
    : _context(context), _info(info) {
    correct_texture_create_info();

    _image_view_of_levels.resize(_info.mip_levels);

    init();

    _bindless_id = _context->bindless_resources()->make_resident(this);
}

Texture::~Texture() {
    _context->bindless_resources()->make_none_resident(this);
    if (_sampler != VK_NULL_HANDLE) {
        _context->device()->Destroy(_sampler);
    }

    for (auto &view : _image_view_of_levels) {
        if (view != VK_NULL_HANDLE) {
            _context->device()->Destroy(view);
        }
    }

    if (_image_view != VK_NULL_HANDLE) {
        _context->device()->Destroy(_image_view);
    }

    if (_image != VK_NULL_HANDLE) {
        vmaDestroyImage(_context->vma()->raw(), _image, _allocation);
    }
}

VkImageSubresourceRange Texture::subresource_range() const {
    return subresource_range(0, _info.mip_levels);
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
        auto stage_buffer = _context->create_buffer(
            size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY);
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
        region.imageSubresource.aspectMask = _info.aspect_mask;
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

void Texture::generate_mipmap(CommandBuffer *cmd,
                              VkPipelineStageFlags src_stage_mask,
                              VkAccessFlags src_access_mask,
                              VkPipelineStageFlags dst_stage_mask,
                              VkAccessFlags dst_access_mask) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Generate Mipmap", 0xFF648123);

    VkFormatProperties format_properties;
    _context->instance()->GetPhysicalDeviceFormatProperties(
        _context->device()->physical_device(),
        _info.format,
        &format_properties);

    if (!(format_properties.optimalTilingFeatures &
          VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT)) {
        ARS_LOG_ERROR("Texture image format does not support linear blit, skip "
                      "mipmap generation");
        return;
    }

    if (_layout == VK_IMAGE_LAYOUT_UNDEFINED) {
        ARS_LOG_ERROR("Texture is not initialized, skip mipmap generation");
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

        barrier.subresourceRange = subresource_range();
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

    auto mip_levels = static_cast<int>(_info.mip_levels);
    assert(mip_levels > 1);
    // transfer all levels to TRANSFER_DST_OPTIMAL
    {
        auto barrier = get_barrier(0,
                                   mip_levels,
                                   _layout,
                                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                   src_access_mask,
                                   VK_ACCESS_TRANSFER_WRITE_BIT);
        cmd->PipelineBarrier(src_stage_mask,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);
    }

    auto mip_width = _info.extent.width;
    auto mip_height = _info.extent.height;
    auto mip_depth = _info.extent.depth;
    for (int i = 1; i < mip_levels; i++) {
        auto next_mip_width = calculate_next_mip_size(mip_width);
        auto next_mip_height = calculate_next_mip_size(mip_height);
        auto next_mip_depth = calculate_next_mip_size(mip_depth);

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
        res.aspectMask = subresource_range().aspectMask;
        res.baseArrayLayer = 0;
        res.layerCount = _info.array_layers;

        VkImageBlit region{};
        region.srcSubresource = res;
        region.srcSubresource.mipLevel = i - 1;
        region.srcOffsets[0] = {0, 0, 0};
        region.srcOffsets[1] = {static_cast<int32_t>(mip_width),
                                static_cast<int32_t>(mip_height),
                                static_cast<int32_t>(mip_depth)};
        region.dstSubresource = res;
        region.dstSubresource.mipLevel = i;
        region.dstOffsets[0] = {0, 0, 0};
        region.dstOffsets[1] = {static_cast<int32_t>(next_mip_width),
                                static_cast<int32_t>(next_mip_height),
                                static_cast<int32_t>(next_mip_depth)};

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
                    dst_access_mask),
        get_barrier(mip_levels - 1,
                    1,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    _layout,
                    VK_ACCESS_TRANSFER_WRITE_BIT,
                    dst_access_mask)};
    cmd->PipelineBarrier(VK_PIPELINE_STAGE_TRANSFER_BIT,
                         dst_stage_mask,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         2,
                         barriers);
}

void Texture::transfer_layout(CommandBuffer *cmd,
                              VkImageLayout dst_layout,
                              VkShaderStageFlags src_stage_mask,
                              VkAccessFlags src_access_mask,
                              VkShaderStageFlags dst_stage_mask,
                              VkAccessFlags dst_access_mask) {
    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};

    auto queue = _context->queue();
    barrier.subresourceRange = subresource_range();
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

VkSampler Texture::sampler() const {
    return _sampler;
}

VkImageLayout Texture::layout() const {
    return _layout;
}

VkImageView Texture::image_view() const {
    return _image_view;
}

const TextureCreateInfo &Texture::info() const {
    return _info;
}

void Texture::assure_layout(VkImageLayout layout) {
    _layout = layout;
}

VkImage Texture::image() const {
    return _image;
}

VkImageView Texture::image_view_of_level(uint32_t level) {
    if (level >= _info.mip_levels) {
        ARS_LOG_ERROR(
            "Image level out of range: Mip level count {}, but you request {}",
            _info.mip_levels,
            level);
        return VK_NULL_HANDLE;
    }

    assert(level < _image_view_of_levels.size());
    auto &view = _image_view_of_levels[level];
    if (view == VK_NULL_HANDLE) {
        view = create_image_view(level, 1);
    }
    return view;
}

VkImageSubresourceRange Texture::subresource_range(uint32_t base_mip_level,
                                                   uint32_t level_count) const {
    VkImageSubresourceRange range{};
    range.aspectMask = _info.aspect_mask;
    range.baseArrayLayer = 0;
    range.layerCount = _info.array_layers;
    range.baseMipLevel = base_mip_level;
    range.levelCount = level_count;

    return range;
}

VkImageView Texture::create_image_view(uint32_t base_mip_level,
                                       uint32_t mip_level_count) {
    VkImageViewCreateInfo view_info{VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
    view_info.image = _image;
    view_info.format = _info.format;
    view_info.viewType = _info.view_type;
    view_info.subresourceRange =
        subresource_range(base_mip_level, mip_level_count);
    view_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    view_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

    VkImageView view = VK_NULL_HANDLE;
    if (_context->device()->Create(&view_info, &view) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create image view");
        return VK_NULL_HANDLE;
    }
    return view;
}

Context *Texture::context() const {
    return _context;
}

void Texture::init() {
    VkImageCreateInfo image_info{VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
    image_info.arrayLayers = _info.array_layers;
    image_info.imageType = _info.image_type;
    image_info.mipLevels = _info.mip_levels;
    image_info.samples = _info.samples;
    image_info.extent = _info.extent;
    image_info.format = _info.format;
    image_info.usage = _info.usage;

    image_info.initialLayout = _layout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;

    if (_info.view_type == VK_IMAGE_VIEW_TYPE_CUBE ||
        _info.view_type == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY) {
        image_info.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    // All textures are used exclusively.
    // Concurrent access disables some potential optimizations from driver.
    // (Like Delta Color Compression(DCC) from AMD drivers, see
    // https://www.khronos.org/assets/uploads/developers/presentations/06-Optimising-aaa-vulkan-title-on-desktop-May19.pdf)
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo alloc_info{};
    alloc_info.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    auto vma = _context->vma();
    if (vmaCreateImage(vma->raw(),
                       &image_info,
                       &alloc_info,
                       &_image,
                       &_allocation,
                       nullptr) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create image");
    }

    _image_view = create_image_view(0, _info.mip_levels);

    VkSamplerCreateInfo sampler_info{VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};

    sampler_info.addressModeU = _info.address_mode_u;
    sampler_info.addressModeV = _info.address_mode_v;
    sampler_info.addressModeW = _info.address_mode_w;
    sampler_info.magFilter = _info.mag_filter;
    sampler_info.minFilter = _info.min_filter;
    sampler_info.mipmapMode = _info.mipmap_mode;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = static_cast<float>(_info.mip_levels);

    auto &context_info = _context->info();
    if (context_info.features.samplerAnisotropy) {
        sampler_info.anisotropyEnable = VK_TRUE;
        sampler_info.maxAnisotropy =
            context_info.properties.limits.maxSamplerAnisotropy;
    }

    if (_context->device()->Create(&sampler_info, &_sampler)) {
        ARS_LOG_CRITICAL("Failed to create sampler");
    }

    _context->set_debug_name(_info.name, _image);
    _context->set_debug_name(_info.name, _image_view);
    _context->set_debug_name(_info.name, _sampler);
}

void Texture::generate_mipmap(const Handle<Texture> &tex, RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.access(tex,
                           VK_ACCESS_TRANSFER_WRITE_BIT |
                               VK_ACCESS_TRANSFER_READ_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT);
        },
        [=](CommandBuffer *cmd) {
            // Externally synchronized
            tex->generate_mipmap(cmd,
                                 VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                 0,
                                 VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                 0);
        });
}

void Texture::correct_texture_create_info() {
    _info.mip_levels = std::clamp(calculate_mip_levels(_info.extent.width,
                                                       _info.extent.height,
                                                       _info.extent.depth),
                                  1u,
                                  _info.mip_levels);

    auto inst = _context->instance();

    VkFormatProperties properties{};
    inst->GetPhysicalDeviceFormatProperties(
        _context->device()->physical_device(), _info.format, &properties);
    _info.usage = remove_unsupported_image_usages(
        _info.usage, properties.optimalTilingFeatures);
}

int32_t Texture::bindless_id() const {
    return _bindless_id;
}

TextureCreateInfo
TextureCreateInfo::sampled_2d(VkFormat format,
                              uint32_t width,
                              uint32_t height,
                              uint32_t mip_levels,
                              VkSamplerAddressMode address_mode) {
    return sampled(VK_IMAGE_TYPE_2D,
                   VK_IMAGE_VIEW_TYPE_2D,
                   format,
                   width,
                   height,
                   1,
                   mip_levels,
                   address_mode);
}

TextureCreateInfo
TextureCreateInfo::sampled_3d(VkFormat format,
                              uint32_t width,
                              uint32_t height,
                              uint32_t depth,
                              uint32_t mip_levels,
                              VkSamplerAddressMode address_mode) {
    return sampled(VK_IMAGE_TYPE_3D,
                   VK_IMAGE_VIEW_TYPE_3D,
                   format,
                   width,
                   height,
                   depth,
                   mip_levels,
                   address_mode);
}

TextureCreateInfo
TextureCreateInfo::sampled(VkImageType image_type,
                           VkImageViewType view_type,
                           VkFormat format,
                           uint32_t width,
                           uint32_t height,
                           uint32_t depth,
                           uint32_t mip_levels,
                           VkSamplerAddressMode address_mode) {
    TextureCreateInfo info{};
    info.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    info.view_type = view_type;
    info.image_type = image_type;
    info.format = format;
    info.usage = IMAGE_USAGE_SAMPLED_COLOR;
    info.samples = VK_SAMPLE_COUNT_1_BIT;
    info.mip_levels = mip_levels;
    info.array_layers = 1;
    info.extent = VkExtent3D{width, height, depth};
    info.address_mode_u = address_mode;
    info.address_mode_v = address_mode;
    info.address_mode_w = address_mode;

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
    _texture->context()->queue()->submit_once([&](CommandBuffer *cmd) {
        _texture->generate_mipmap(cmd,
                                  VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                                  0,
                                  VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                                  0);
    });
}

Handle<Texture> TextureAdapter::texture() const {
    return _texture;
}

ITextureHandle *TextureAdapter::handle() {
    return _texture.get();
}

VkFormat translate(render::Format format) {
    switch (format) {
    case Format::R8_SRGB:
        return VK_FORMAT_R8_SRGB;
    case Format::R8_UNORM:
        return VK_FORMAT_R8_UNORM;
    case Format::R8G8_SRGB:
        return VK_FORMAT_R8G8_SRGB;
    case Format::R8G8_UNORM:
        return VK_FORMAT_R8G8_UNORM;
    case Format::R8G8B8_SRGB:
        return VK_FORMAT_R8G8B8_SRGB;
    case Format::R8G8B8_UNORM:
        return VK_FORMAT_R8G8B8_UNORM;
    case Format::R8G8B8A8_SRGB:
        return VK_FORMAT_R8G8B8A8_SRGB;
    case Format::R8G8B8A8_UNORM:
        return VK_FORMAT_R8G8B8A8_UNORM;
    case Format::R32G32B32A32_SFLOAT:
        return VK_FORMAT_R32G32B32A32_SFLOAT;
    case Format::B10G11R11_UFLOAT_PACK32:
        return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    }
    // should not happen
    return VK_FORMAT_UNDEFINED;
}

VkFilter translate(FilterMode mode) {
    switch (mode) {
    case FilterMode::Linear:
        return VK_FILTER_LINEAR;
    case FilterMode::Nearest:
        return VK_FILTER_NEAREST;
    }
    // should not happen
    return VK_FILTER_LINEAR;
}

VkSamplerMipmapMode translate(MipmapMode mode) {
    switch (mode) {
    case MipmapMode::Linear:
        return VK_SAMPLER_MIPMAP_MODE_LINEAR;
    case MipmapMode::Nearest:
        return VK_SAMPLER_MIPMAP_MODE_NEAREST;
    }
    // should not happen
    return VK_SAMPLER_MIPMAP_MODE_LINEAR;
}

VkSamplerAddressMode translate(WrapMode mode) {
    switch (mode) {
    case WrapMode::Repeat:
        return VK_SAMPLER_ADDRESS_MODE_REPEAT;
    case WrapMode::ClampToEdge:
        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    case WrapMode::MirroredRepeat:
        return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
    }
    // should not happen
    return VK_SAMPLER_ADDRESS_MODE_REPEAT;
}

TextureCreateInfo translate(const TextureInfo &info) {
    TextureCreateInfo up{};
    up.name = info.name;
    up.format = translate(info.format);
    switch (info.type) {
    case TextureType::Texture2D:
        up.image_type = VK_IMAGE_TYPE_2D;
        up.view_type = VK_IMAGE_VIEW_TYPE_2D;
        break;
    case TextureType::CubeMap:
        up.image_type = VK_IMAGE_TYPE_2D;
        up.view_type = VK_IMAGE_VIEW_TYPE_CUBE;
        break;
    }
    up.array_layers = info.array_layers;
    if (info.type == TextureType::CubeMap) {
        up.array_layers = 6 * info.array_layers;
    }
    up.mip_levels = info.mip_levels;
    up.extent.width = info.width;
    up.extent.height = info.height;
    up.extent.depth = info.depth;
    up.aspect_mask = VK_IMAGE_ASPECT_COLOR_BIT;
    up.samples = VK_SAMPLE_COUNT_1_BIT;
    up.usage = IMAGE_USAGE_SAMPLED_COLOR | VK_IMAGE_USAGE_STORAGE_BIT;

    up.min_filter = translate(info.min_filter);
    up.mag_filter = translate(info.mag_filter);
    up.mipmap_mode = translate(info.mipmap_mode);
    up.address_mode_u = translate(info.wrap_u);
    up.address_mode_v = translate(info.wrap_v);
    up.address_mode_w = translate(info.wrap_w);
    return up;
}

Handle<Texture> upcast(ITexture *texture) {
    if (texture == nullptr) {
        return {};
    }

    return dynamic_cast<TextureAdapter *>(texture)->texture();
}

Texture *upcast(ITextureHandle *handle) {
    return dynamic_cast<Texture *>(handle);
}
} // namespace ars::render::vk
