#include "OverlayRenderer.h"
#include "../Context.h"
#include "../Scene.h"
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

    init_forward_render_pass();
    init_billboard_pipeline();
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
    return need_render_billboard() || need_render_outline();
}

void OverlayRenderer::render(CommandBuffer *cmd, NamedRT dst_rt_name) {
    auto rd_outline = need_render_outline();
    auto rd_billboard = need_render_billboard();
    auto dst_rt = _view->render_target(dst_rt_name);
    if (rd_outline) {
        render_outline(cmd, dst_rt);
    }
    if (rd_outline && rd_billboard) {
        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.image = dst_rt->image();
        barrier.subresourceRange = dst_rt->subresource_range();
        barrier.oldLayout = dst_rt->layout();
        barrier.newLayout = dst_rt->layout();
        barrier.srcAccessMask =
            VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |
                                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        cmd->PipelineBarrier(VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                             VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);
    }
    if (rd_billboard) {
        render_billboard(cmd, dst_rt);
    }
}

void OverlayRenderer::ensure_outline_rts() {
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

void OverlayRenderer::set_light_gizmo(const std::shared_ptr<ITexture> &texture,
                                      float width) {
    _light_gizmo.texture = upcast(texture.get());
    _light_gizmo.width = width;
}

LightGizmo OverlayRenderer::light_gizmo() const {
    return _light_gizmo;
}

void OverlayRenderer::init_forward_render_pass() {
    // Final color have same rt info, use any of them for final render should be
    // fine
    auto color_info = _view->rt_info(NamedRT_FinalColor0);
    auto depth_info = _view->rt_info(NamedRT_Depth);
    RenderPassAttachmentInfo color_attach{};
    color_attach.format = color_info.texture.format;
    color_attach.samples = color_info.texture.samples;
    color_attach.initial_layout = VK_IMAGE_LAYOUT_GENERAL;
    color_attach.final_layout = VK_IMAGE_LAYOUT_GENERAL;
    color_attach.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    color_attach.store_op = VK_ATTACHMENT_STORE_OP_STORE;

    RenderPassAttachmentInfo depth_attach{};
    depth_attach.format = depth_info.texture.format;
    depth_attach.samples = depth_info.texture.samples;
    depth_attach.initial_layout = VK_IMAGE_LAYOUT_GENERAL;
    depth_attach.final_layout = VK_IMAGE_LAYOUT_GENERAL;
    depth_attach.load_op = VK_ATTACHMENT_LOAD_OP_LOAD;
    depth_attach.store_op = VK_ATTACHMENT_STORE_OP_STORE;

    _overlay_forward_pass = RenderPass::create_with_single_pass(
        _view->context(), 1, &color_attach, &depth_attach);
}

void OverlayRenderer::init_billboard_pipeline() {
    auto ctx = _view->context();
    auto vert_shader = std::make_unique<Shader>(ctx, "Billboard.vert");
    auto frag_shader = std::make_unique<Shader>(ctx, "Billboard.frag");

    VkPipelineColorBlendAttachmentState attachment_blend =
        create_attachment_blend_state(VK_BLEND_FACTOR_SRC_ALPHA,
                                      VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);
    VkPipelineColorBlendStateCreateInfo blend{
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
    blend.attachmentCount = 1;
    blend.pAttachments = &attachment_blend;

    VkPushConstantRange push_constant{};
    push_constant.size = sizeof(glm::mat4);
    push_constant.offset = 0;
    push_constant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    auto depth_stencil = enabled_depth_stencil_state(false);

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.render_pass = _overlay_forward_pass.get();
    info.depth_stencil = &depth_stencil;
    info.blend = &blend;
    info.push_constant_range_count = 1;
    info.push_constant_ranges = &push_constant;
    info.subpass = 0;

    _billboard_pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
}

void OverlayRenderer::render_outline(CommandBuffer *cmd,
                                     const Handle<Texture> &dst_rt) {
    ensure_outline_rts();
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

    auto id_rp = _view->drawer()->draw_id_render_pass();
    auto id_rp_exec = id_rp->begin(
        cmd, {outline_id, outline_depth}, VK_SUBPASS_CONTENTS_INLINE);
    _view->drawer()->draw_ids(
        cmd, outline_request_count, mvp_arr.data(), ids.data(), meshes.data());
    id_rp->end(id_rp_exec);

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
    desc.set_texture(0, 1, dst_rt.get());
    desc.set_buffer_data(1, 0, param);
    desc.commit(cmd, _outline_detect_pipeline.get());

    _outline_detect_pipeline->local_size().dispatch(
        cmd, extent.width, extent.height, 1);

    _outline_draw_requests.clear();
}

bool OverlayRenderer::need_render_outline() const {
    return !_outline_draw_requests.empty();
}

bool OverlayRenderer::need_render_billboard() const {
    return _light_gizmo.texture.get() != nullptr;
}

void OverlayRenderer::render_billboard(CommandBuffer *cmd,
                                       const Handle<Texture> &dst_rt) {
    auto depth_rt = _view->render_target(NamedRT_Depth);
    auto rp_exec = _overlay_forward_pass->begin(
        cmd, {dst_rt, depth_rt}, VK_SUBPASS_CONTENTS_INLINE);

    _billboard_pipeline->bind(cmd);

    auto tex = _light_gizmo.texture;
    auto tex_extent = tex->info().extent;
    auto width = _light_gizmo.width;
    auto height = width * static_cast<float>(tex_extent.height) /
                  static_cast<float>(tex_extent.width);

    DescriptorEncoder desc{};
    desc.set_texture(0, 0, tex.get());
    desc.commit(cmd, _billboard_pipeline.get());

    auto p_matrix = _view->projection_matrix();

    auto draw_billboard = [&](const glm::vec3 &pos) {
        glm::mat4 mvp =
            p_matrix * _view->billboard_MV_matrix(pos, width, height);
        cmd->PushConstants(_billboard_pipeline->pipeline_layout(),
                           VK_SHADER_STAGE_VERTEX_BIT,
                           0,
                           sizeof(glm::mat4),
                           &mvp);
        cmd->Draw(6, 1, 0, 0);
    };

    auto draw_light_icons = [&](auto &&lights) {
        auto xforms = lights.template get_array<math::XformTRS<float>>();
        auto light_count = lights.size();
        for (int i = 0; i < light_count; i++) {
            draw_billboard(xforms[i].translation());
        }
    };

    auto point_lights = _view->vk_scene()->point_lights;
    draw_light_icons(point_lights);
    auto directional_lights = _view->vk_scene()->directional_lights;
    draw_light_icons(directional_lights);

    _overlay_forward_pass->end(rp_exec);
}
} // namespace ars::render::vk