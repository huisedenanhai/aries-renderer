#include "Shadow.h"
#include "../Context.h"
#include "../Profiler.h"
#include "../Scene.h"
#include "Drawer.h"

namespace ars::render::vk {
Shadow::Shadow(View *view) : _view(view) {}

void Shadow::render(RenderGraph &rg, const CullingResult &culling_result) {
    auto &dir_lights = _view->scene_vk()->directional_lights;
    auto light_cnt = dir_lights.size();
    auto shadow_maps = dir_lights.get_array<std::unique_ptr<ShadowMap>>();
    auto xform_arr = dir_lights.get_array<math::XformTRS<float>>();
    for (int i = 0; i < light_cnt; i++) {
        auto sm = shadow_maps[i].get();
        if (sm != nullptr) {
            sm->render(xform_arr[i], rg, culling_result);
        }
    }
}

ShadowMap::ShadowMap(Context *context, uint32_t resolution)
    : _context(context) {
    auto sm_info =
        TextureCreateInfo::sampled_2d(RT_FORMAT_DEPTH,
                                      resolution,
                                      resolution,
                                      1,
                                      VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER);
    sm_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT |
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    sm_info.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
    _texture = _context->create_texture(sm_info);
}

void ShadowMap::render(const math::XformTRS<float> &xform,
                       RenderGraph &rg,
                       const CullingResult &culling_result) {
    auto view = rg.view();
    auto scene = view->scene_vk();

    update_camera(xform, scene, culling_result);

    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.access(_texture,
                           VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                           VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT |
                               VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
        },
        [=](CommandBuffer *cmd) {
            ARS_PROFILER_SAMPLE_VK(cmd, "Shadow", 0xFF394139);
            auto ctx = view->context();
            auto rp =
                ctx->renderer_data()->subpass(RenderPassID_Shadow).render_pass;
            Framebuffer *fb = ctx->create_tmp_framebuffer(rp, {_texture});

            // clear all rts to zero
            VkClearValue clear_values[1]{};
            auto rp_exec =
                rp->begin(cmd, fb, clear_values, VK_SUBPASS_CONTENTS_INLINE);

            auto shadow_cull_obj = scene->cull(_xform, _camera.frustum(1.0f));

            auto draw_requests =
                shadow_cull_obj.gather_draw_requests(RenderPassID_Shadow);

            DrawCallbacks callbacks{};
            callbacks.on_pipeline_bound = [&](CommandBuffer *cmd) {
                // Negative sign to bias because we use inverse z
                cmd->SetDepthBias(-_constant_bias, 0.0f, -_slope_bias);
            };
            view->drawer()->draw(cmd,
                                 _camera.projection_matrix(1.0f),
                                 glm::inverse(_xform.matrix_no_scale()),
                                 draw_requests,
                                 callbacks);

            rp->end(rp_exec);
        });
}

void ShadowMap::update_camera(const math::XformTRS<float> &xform,
                              Scene *scene,
                              const CullingResult &culling_result) {
    auto ls_to_ws = xform.matrix_no_scale();
    auto ws_to_ls = glm::inverse(ls_to_ws);
    auto scene_aabb_ls =
        math::transform_aabb(ws_to_ls, scene->loaded_aabb_ws());
    auto visible_aabb_ls =
        math::transform_aabb(ws_to_ls, culling_result.visible_aabb_ws);
    auto visible_center_ls = visible_aabb_ls.center();
    auto visible_extent_ls = visible_aabb_ls.extent();

    // Make near plane fits scene aabb
    auto z_near = 0.01f;
    auto light_z = scene_aabb_ls.max.z + z_near;
    // Make far plane contains visible aabb
    auto z_far = light_z - visible_aabb_ls.min.z;

    auto light_center_ls =
        glm::vec3(visible_center_ls.x, visible_center_ls.y, light_z);
    auto light_center_ws = math::transform_position(ls_to_ws, light_center_ls);

    // Update fields
    _xform = xform;
    _xform.set_translation(light_center_ws);

    _camera.z_near = z_near;
    _camera.z_far = z_far;
    _camera.y_mag = std::max(visible_extent_ls.x, visible_extent_ls.y) * 0.5f;
}

Handle<Texture> ShadowMap::texture() const {
    return _texture;
}

ShadowData ShadowMap::data(View *view) const {
    ShadowData d{};
    auto PS = _camera.projection_matrix(1.0f);
    auto VS = glm::inverse(_xform.matrix_no_scale());
    auto I_V = glm::inverse(view->view_matrix());

    d.view_to_shadow_hclip = PS * VS * I_V;

    return d;
}

} // namespace ars::render::vk