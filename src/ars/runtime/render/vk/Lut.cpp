#include "Lut.h"
#include "Context.h"
#include "Pipeline.h"
#include "Texture.h"

namespace ars::render::vk {
Lut::Lut(Context *context) : _context(context) {
    init_brdf_lut();
}

Handle<Texture> Lut::brdf_lut() const {
    return _brdf_lut;
}

void Lut::init_brdf_lut() {
    int32_t width = 64;
    int32_t height = 64;
    auto tex_info =
        TextureCreateInfo::sampled_2d(VK_FORMAT_R16G16B16A16_SFLOAT,
                                      width,
                                      height,
                                      1,
                                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
    tex_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;
    _brdf_lut = _context->create_texture(tex_info);

    auto pipeline = ComputePipeline::create(_context, "BRDFLut.comp");

    _context->queue()->submit_once([&](CommandBuffer *cmd) {
        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.image = _brdf_lut->image();
        barrier.subresourceRange = _brdf_lut->subresource_range();

        cmd->PipelineBarrier(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
                             VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        _brdf_lut->assure_layout(VK_IMAGE_LAYOUT_GENERAL);

        pipeline->bind(cmd);

        struct Param {
            int32_t width;
            int32_t height;
        };
        DescriptorEncoder desc{};

        Param param{};
        param.width = width;
        param.height = height;

        desc.set_buffer_data(1, 0, param);
        desc.set_texture(0, 0, _brdf_lut.get());
        desc.commit(cmd, pipeline.get());

        pipeline->local_size().dispatch(cmd, width, height, 1);
    });

    // Wait execution finish so the tmp pipeline can be destroyed.
    _context->queue()->flush();
}

Lut::~Lut() = default;
} // namespace ars::render::vk
