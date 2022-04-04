#include "Shadow.h"
#include "../Context.h"
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
                cmd->SetDepthBias(0.0f, 0.0f, -_slope_bias);
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
    auto scene_aabb = scene->loaded_aabb_ws();
    auto visible_aabb = culling_result.visible_aabb_ws;
    auto visible_center = visible_aabb.center();
    auto visible_radius = visible_aabb.radius();
    auto scene_center = scene_aabb.center();
    auto scene_radius = scene_aabb.radius();
    auto light_backward = -glm::normalize(xform.forward());

    // Make near plane externally tangent to scene bounding sphere
    auto z_near = 0.01f;
    auto light_from_visible_center =
        scene_radius - glm::dot(light_backward, visible_center - scene_center) +
        z_near;

    auto light_center =
        visible_center + light_backward * light_from_visible_center;

    // Make far plane externally tangent to visible bounding sphere
    auto z_far = light_from_visible_center + visible_radius;

    // Update fields
    _xform = xform;
    _xform.set_translation(light_center);

    _camera.z_near = z_near;
    _camera.z_far = z_far;
    _camera.y_mag = visible_radius;
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