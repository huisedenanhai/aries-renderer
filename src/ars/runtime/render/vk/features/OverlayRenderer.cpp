#include "OverlayRenderer.h"
#include "Drawer.h"

namespace ars::render::vk {
namespace {
uint32_t encode_outline_id(uint8_t group, uint32_t index) {
    return (index << 8) | group;
}
} // namespace

OverlayRenderer::OverlayRenderer(View *view) : _view(view) {
    std::fill(_outline_colors.begin(),
              _outline_colors.end(),
              glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

    _outline_detect_pipeline =
        ComputePipeline::create(_view->context(), "Outline.comp");
}

glm::vec4 OverlayRenderer::outline_color(uint8_t group) {
    return _outline_colors[group];
}

void OverlayRenderer::set_outline_color(uint8_t group, const glm::vec4 &color) {
    _outline_colors[group] = color;
}

void OverlayRenderer::draw_outline(uint8_t group,
                                   const math::XformTRS<float> &xform,
                                   const std::shared_ptr<IMesh> &mesh) {
    OutlineDrawRequest request{};
    request.group = group;
    request.xform = xform;
    request.mesh = upcast(mesh);
    _outline_draw_requests.emplace_back(std::move(request));
}

bool OverlayRenderer::need_render() const {
    return !_outline_draw_requests.empty();
}

void OverlayRenderer::render(CommandBuffer *cmd, NamedRT dst_rt) {
    ensure_rts();
    auto rt_manager = _view->rt_manager();

    auto p_matrix = _view->projection_matrix();
    auto v_matrix = _view->view_matrix();

    std::vector<uint32_t> ids{};
    std::vector<glm::mat4> mvp_arr{};
    std::vector<std::shared_ptr<Mesh>> meshes{};

    auto outline_request_count = _outline_draw_requests.size();

    ids.reserve(outline_request_count);
    mvp_arr.reserve(outline_request_count);
    meshes.reserve(outline_request_count);

    for (int i = 0; i < outline_request_count; i++) {
        auto &req = _outline_draw_requests[i];
        ids.push_back(encode_outline_id(req.group, i + 1));
        mvp_arr.push_back(p_matrix * v_matrix * req.xform.matrix());
        meshes.push_back(req.mesh);
    }

    auto outline_id = rt_manager->get(_outline_id_rt);
    auto outline_depth = rt_manager->get(_outline_depth_rt);

    _view->drawer()->draw_ids(cmd,
                              outline_id,
                              outline_depth,
                              outline_request_count,
                              mvp_arr.data(),
                              ids.data(),
                              meshes.data());

    VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
    barrier.image = outline_id->image();
    barrier.subresourceRange = outline_id->subresource_range();
    barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    cmd->PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                         0,
                         0,
                         nullptr,
                         0,
                         nullptr,
                         1,
                         &barrier);

    struct Param {
        int width;
        int height;
        ARS_PADDING_FIELD(float);
        ARS_PADDING_FIELD(float);
        glm::vec4 color[256];
    };

    auto extent = outline_id->info().extent;
    Param param{};
    param.width = static_cast<int>(extent.width);
    param.height = static_cast<int>(extent.height);
    assert(_outline_colors.size() == std::size(param.color));
    std::memcpy(param.color,
                _outline_colors.data(),
                sizeof(glm::vec4) * std::size(param.color));

    _outline_detect_pipeline->bind(cmd);

    DescriptorEncoder desc{};
    desc.set_texture(0, 0, outline_id.get());
    desc.set_texture(0, 1, _view->render_target(dst_rt).get());
    desc.set_buffer_data(1, 0, param);
    desc.commit(cmd, _outline_detect_pipeline.get());

    _outline_detect_pipeline->local_size().dispatch(
        cmd, extent.width, extent.height, 1);

    _outline_draw_requests.clear();
}

void OverlayRenderer::ensure_rts() {
    if (!_outline_id_rt.valid()) {
        RenderTargetInfo info{};
        auto &tex = info.texture;
        tex =
            TextureCreateInfo::sampled_2d(ID_COLOR_ATTACHMENT_FORMAT, 1, 1, 1);
        tex.usage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        tex.mag_filter = VK_FILTER_NEAREST;
        tex.min_filter = VK_FILTER_NEAREST;
        _outline_id_rt = _view->rt_manager()->alloc(info);
    }

    if (!_outline_depth_rt.valid()) {
        RenderTargetInfo info{};
        info.texture = TextureCreateInfo::sampled_2d(
            ID_DEPTH_STENCIL_ATTACHMENT_FORMAT, 1, 1, 1);
        info.texture.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
        info.texture.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        _outline_depth_rt = _view->rt_manager()->alloc(info);
    }
}

OverlayRenderer::~OverlayRenderer() {
    auto rt_manager = _view->rt_manager();
    rt_manager->free(_outline_id_rt);
    rt_manager->free(_outline_depth_rt);
}
} // namespace ars::render::vk