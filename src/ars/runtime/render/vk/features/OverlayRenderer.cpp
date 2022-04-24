#include "OverlayRenderer.h"
#include "../Context.h"
#include "../Profiler.h"
#include "../Scene.h"
#include "Drawer.h"
#include "Renderer.h"

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

    init_billboard_pipeline();
    init_line_pipeline();
}

glm::vec4 OverlayRenderer::outline_color(uint8_t group) {
    return _outline_colors[group];
}

void OverlayRenderer::set_outline_color(uint8_t group, const glm::vec4 &color) {
    _outline_colors[group] = color;
}

void OverlayRenderer::render(RenderGraph &rg, NamedRT dst_rt_name) {
    auto rd_outline = need_render_outline();
    auto rd_forward = need_forward_pass();
    auto dst_rt = _view->render_target(dst_rt_name);
    if (rd_outline) {
        // Draw object ids
        ensure_outline_rts();
        rg.add_pass(
            [&](RenderGraphPassBuilder &builder) {
                builder.access(_outline_depth_rt,
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
                builder.access(_outline_id_rt,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
            },
            [=](CommandBuffer *cmd) { render_object_ids(cmd); });

        // Calculate outline based on object ids
        rg.add_pass(
            [&](RenderGraphPassBuilder &builder) {
                builder.compute_shader_read(_outline_id_rt);
                builder.compute_shader_read_write(dst_rt);
            },
            [=](CommandBuffer *cmd) { calculate_outline(cmd, dst_rt); });
    }
    if (rd_forward) {
        rg.add_pass(
            [&](RenderGraphPassBuilder &builder) {
                builder.access(dst_rt,
                               VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                   VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
                               VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
                // Outline and billboard are all rendered transparently and does
                // not write depth
                builder.access(NamedRT_Depth,
                               VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
                               VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
            },
            [=](CommandBuffer *cmd) {
                ARS_PROFILER_SAMPLE_VK(cmd, "Overlay Forward", 0xFF891742);
                auto rp = overlay_pass().render_pass;
                auto depth_rt = _view->render_target(NamedRT_Depth);
                auto rp_exec = rp->begin(
                    cmd, {dst_rt, depth_rt}, VK_SUBPASS_CONTENTS_INLINE);
                render_line(cmd);
                render_billboard(cmd);
                rp->end(rp_exec);
            });
    }
}

void OverlayRenderer::ensure_outline_rts() {
    if (!_outline_id_rt.valid()) {
        TextureCreateInfo info = TextureCreateInfo::sampled_2d(
            ID_COLOR_ATTACHMENT_FORMAT,
            1,
            1,
            1,
            VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE);
        info.usage =
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        info.mag_filter = VK_FILTER_NEAREST;
        info.min_filter = VK_FILTER_NEAREST;
        _outline_id_rt = _view->rt_manager()->alloc(info);
    }

    if (!_outline_depth_rt.valid()) {
        TextureCreateInfo info =
            TextureCreateInfo::sampled_2d(RT_FORMAT_DEPTH, 1, 1, 1);
        info.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
        info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
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

void OverlayRenderer::init_billboard_pipeline() {
    auto ctx = _view->context();
    auto vert_shader = Shader::find_precompiled(ctx, "Billboard.vert");
    auto frag_shader = Shader::find_precompiled(ctx, "Billboard.frag");

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
    info.subpass = overlay_pass();
    info.depth_stencil = &depth_stencil;
    info.blend = &blend;
    info.push_constant_range_count = 1;
    info.push_constant_ranges = &push_constant;

    _billboard_pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
}

void OverlayRenderer::calculate_outline(CommandBuffer *cmd,
                                        const Handle<Texture> &dst_rt) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Calculate Outline", 0xFF857191);
    auto outline_id = _view->rt_manager()->get(_outline_id_rt);
    struct Param {
        glm::vec4 color[256];
    };

    auto extent = outline_id->info().extent;
    Param param{};
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

void OverlayRenderer::render_billboard(CommandBuffer *cmd) {
    if (!need_render_billboard()) {
        return;
    }
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

    auto &point_lights = _view->scene_vk()->point_lights;
    draw_light_icons(point_lights);
    auto &directional_lights = _view->scene_vk()->directional_lights;
    draw_light_icons(directional_lights);
}

void OverlayRenderer::draw_line(const glm::vec3 &from,
                                const glm::vec3 &to,
                                const glm::vec4 &color) {
    _line_vert_pos.push_back(from);
    _line_vert_pos.push_back(to);
    _line_vert_color.push_back(color);
    _line_vert_color.push_back(color);
}

void OverlayRenderer::init_line_pipeline() {
    auto ctx = _view->context();
    auto vert_shader = Shader::find_precompiled(ctx, "VertexColor.vert");
    auto frag_shader = Shader::find_precompiled(ctx, "VertexColor.frag");

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

    VkPipelineVertexInputStateCreateInfo vertex_input{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkVertexInputBindingDescription vert_bindings[2] = {
        {0,
         static_cast<uint32_t>(sizeof(glm::vec3)),
         VK_VERTEX_INPUT_RATE_VERTEX},
        {1,
         static_cast<uint32_t>(sizeof(glm::vec4)),
         VK_VERTEX_INPUT_RATE_VERTEX},
    };

    VkVertexInputAttributeDescription vert_attrs[2] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
        {1, 1, VK_FORMAT_R32G32B32A32_SFLOAT, 0},
    };

    vertex_input.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(std::size(vert_attrs));
    vertex_input.pVertexAttributeDescriptions = vert_attrs;

    vertex_input.vertexBindingDescriptionCount =
        static_cast<uint32_t>(std::size(vert_bindings));
    vertex_input.pVertexBindingDescriptions = vert_bindings;

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.subpass = overlay_pass();
    info.vertex_input = &vertex_input;
    info.depth_stencil = &depth_stencil;
    info.blend = &blend;
    info.push_constant_range_count = 1;
    info.push_constant_ranges = &push_constant;
    info.topology = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;

    _line_pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
}

bool OverlayRenderer::need_render_line() const {
    return !_line_vert_pos.empty();
}

void OverlayRenderer::render_line(CommandBuffer *cmd) {
    ARS_DEFER([&]() {
        _line_vert_pos.clear();
        _line_vert_color.clear();
    });
    if (!need_render_line()) {
        return;
    }

    _line_pipeline->bind(cmd);
    auto vert_count = _line_vert_pos.size();
    assert(vert_count == _line_vert_color.size());
    auto ctx = _view->context();
    auto position_buffer = ctx->create_buffer(sizeof(glm::vec3) * vert_count,
                                              VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                              VMA_MEMORY_USAGE_CPU_TO_GPU);
    auto color_buffer = ctx->create_buffer(sizeof(glm::vec4) * vert_count,
                                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                                           VMA_MEMORY_USAGE_CPU_TO_GPU);

    position_buffer->map_once([&](void *ptr) {
        std::memcpy(ptr, _line_vert_pos.data(), sizeof(glm::vec3) * vert_count);
    });
    color_buffer->map_once([&](void *ptr) {
        std::memcpy(
            ptr, _line_vert_color.data(), sizeof(glm::vec4) * vert_count);
    });

    VkBuffer vert_buffers[2] = {position_buffer->buffer(),
                                color_buffer->buffer()};
    VkDeviceSize offset[2] = {};

    cmd->BindVertexBuffers(0, 2, vert_buffers, offset);

    glm::mat4 mvp = _view->projection_matrix() * _view->view_matrix();
    cmd->PushConstants(_line_pipeline->pipeline_layout(),
                       VK_SHADER_STAGE_VERTEX_BIT,
                       0,
                       sizeof(glm::mat4),
                       &mvp);

    cmd->Draw(vert_count, 1, 0, 0);
}

bool OverlayRenderer::need_forward_pass() const {
    return need_render_billboard() || need_render_line();
}

void OverlayRenderer::render_object_ids(CommandBuffer *cmd) {
    ARS_PROFILER_SAMPLE_VK(cmd, "Outline Object ID", 0xFF173752);
    auto rt_manager = _view->rt_manager();

    auto p_matrix = _view->projection_matrix();
    auto v_matrix = _view->view_matrix();

    auto outline_id = rt_manager->get(_outline_id_rt);
    auto outline_depth = rt_manager->get(_outline_depth_rt);

    auto id_rp = _view->drawer()->draw_id_render_pass();
    auto id_rp_exec = id_rp->begin(
        cmd, {outline_id, outline_depth}, VK_SUBPASS_CONTENTS_INLINE);
    _view->drawer()->draw(cmd, _outline_draw_requests);
    id_rp->end(id_rp_exec);
}

SubpassInfo OverlayRenderer::overlay_pass() const {
    return _view->context()->renderer_data()->subpass(RenderPassID_Overlay);
}

void OverlayRenderer::draw_outline(uint8_t group, IRenderObject *rd_object) {
    auto obj_id = upcast(rd_object)->id();
    auto scene = _view->scene_vk();
    auto req = scene->get_draw_request(RenderPassID_ObjectID, obj_id);
    if (!req.has_value()) {
        // Use a fallback when outline material is not specified
        req = scene->get_draw_request(
            RenderPassID_ObjectID,
            obj_id,
            _view->context()->default_material_vk().get());
    }
    if (req.has_value()) {
        add_outline_draw_request(group, req.value());
    }
}

void OverlayRenderer::add_outline_draw_request(uint8_t group,
                                               const DrawRequest &req) {
    auto r = req;
    r.custom_id = encode_outline_id(group, _outline_draw_requests.size() + 1);
    _outline_draw_requests.push_back(r);
}
} // namespace ars::render::vk