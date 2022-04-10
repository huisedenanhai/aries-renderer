#include "Shadow.h"
#include "../Context.h"
#include "../Profiler.h"
#include "../Scene.h"
#include "Drawer.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
Shadow::Shadow(View *view) : _view(view) {}

void Shadow::render(RenderGraph &rg, const CullingResult &culling_result) {
    auto sample_dist = calculate_sample_distribution();

    auto &dir_lights = _view->scene_vk()->directional_lights;
    auto light_cnt = dir_lights.size();
    auto shadow_maps = dir_lights.get_array<std::unique_ptr<ShadowMap>>();
    auto xform_arr = dir_lights.get_array<math::XformTRS<float>>();
    for (int i = 0; i < light_cnt; i++) {
        auto sm = shadow_maps[i].get();
        if (sm != nullptr) {
            sm->render(xform_arr[i], rg, culling_result, sample_dist);
        }
    }
}

void Shadow::read_back_hiz(RenderGraph &rg) {
    rg.add_pass(
        [&](RenderGraphPassBuilder &builder) {
            builder.access(NamedRT_HiZBuffer,
                           VK_ACCESS_TRANSFER_READ_BIT,
                           VK_PIPELINE_STAGE_TRANSFER_BIT,
                           false,
                           VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
            builder.has_side_effect(true);
        },
        [=](CommandBuffer *cmd) {
            ARS_PROFILER_SAMPLE_VK(cmd, "Read Back HiZ", 0xFF17B13A);
            auto hiz = _view->render_target(NamedRT_HiZBuffer);
            auto hiz_info = hiz->info();
            // Calculate level size
            _hiz_buffer_width = std::max(
                1u, hiz_info.extent.width >> _hiz_buffer_capture_level);
            _hiz_buffer_height = std::max(
                1u, hiz_info.extent.height >> _hiz_buffer_capture_level);
            auto level =
                std::min(hiz_info.mip_levels - 1, _hiz_buffer_capture_level);

            // Reserve buffer space
            auto buffer_size_in_bytes =
                _hiz_buffer_width * _hiz_buffer_height * 2 * sizeof(float);
            if (_last_frame_hiz_data == nullptr ||
                _last_frame_hiz_data->size() < buffer_size_in_bytes) {
                _last_frame_hiz_data = _view->context()->create_buffer(
                    buffer_size_in_bytes,
                    VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                    VMA_MEMORY_USAGE_GPU_TO_CPU);
            }

            // Do copy
            VkBufferImageCopy region{};
            region.imageExtent.width = _hiz_buffer_width;
            region.imageExtent.height = _hiz_buffer_height;
            region.imageExtent.depth = 1;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.baseArrayLayer = 0;
            region.imageSubresource.layerCount = 1;
            region.imageSubresource.mipLevel = level;

            cmd->CopyImageToBuffer(hiz->image(),
                                   hiz->layout(),
                                   _last_frame_hiz_data->buffer(),
                                   1,
                                   &region);
        });
}

SampleDistribution Shadow::calculate_sample_distribution() {
    ARS_PROFILER_SAMPLE("Calculate Depth Sample Dist", 0xFF999411);
    if (_last_frame_hiz_data == nullptr) {
        return {};
    }

    SampleDistribution dist{};

    _last_frame_hiz_data->map_once([&](void *ptr) {
        auto hiz_pixels = reinterpret_cast<glm::vec2 *>(ptr);
        auto min_depth01 = 1.0f;
        auto max_depth01 = 0.0f;
        for (int y = 0; y < _hiz_buffer_height; y++) {
            for (int x = 0; x < _hiz_buffer_width; x++) {
                auto i = y * _hiz_buffer_width + x;
                auto depth01 = hiz_pixels[i];
                assert(depth01.x >= depth01.y);
                max_depth01 = std::max(max_depth01, depth01.x);
                if (depth01.y != 0) {
                    min_depth01 = std::min(min_depth01, depth01.y);
                }
            }
        }
        // At this point, min_depth01 == 1.0f when
        // 1. Nothing is rendered on screen
        // 2. All fragments are snapped to the near plane
        if (min_depth01 == 1.0f && max_depth01 < min_depth01) {
            min_depth01 = 0.0f;
        }
        assert(min_depth01 <= max_depth01);

        auto last_frame_P = _view->last_frame_projection_matrix();
        auto last_frame_I_P = glm::inverse(last_frame_P);
        dist.z_near = depth01_to_linear_z(last_frame_I_P, max_depth01);
        dist.z_far = depth01_to_linear_z(last_frame_I_P, min_depth01);
    });

    return dist;
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
                       const CullingResult &culling_result,
                       const SampleDistribution &sample_dist) {
    auto view = rg.view();
    auto scene = view->scene_vk();

    update_cascade_cameras(xform, view, sample_dist);

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

            // Render each cascades
            for (auto &cam : _cascade_cameras) {
                fb->set_viewport_scissor(cmd,
                                         cam.viewport.x,
                                         cam.viewport.y,
                                         cam.viewport.w,
                                         cam.viewport.h);

                float w_div_h = cam.viewport.w / cam.viewport.h;
                auto shadow_cull_obj =
                    scene->cull(cam.xform, cam.camera.frustum(w_div_h));

                auto draw_requests =
                    shadow_cull_obj.gather_draw_requests(RenderPassID_Shadow);

                DrawCallbacks callbacks{};
                callbacks.on_pipeline_bound = [&](CommandBuffer *cmd) {
                    // Negative sign to bias because we use inverse z
                    cmd->SetDepthBias(-_constant_bias, 0.0f, -_slope_bias);
                };

                view->drawer()->draw(cmd,
                                     cam.camera.projection_matrix(w_div_h),
                                     glm::inverse(cam.xform.matrix_no_scale()),
                                     draw_requests,
                                     callbacks);
            }

            rp->end(rp_exec);
        });
}

void ShadowMap::update_cascade_cameras(const math::XformTRS<float> &xform,
                                       View *view,
                                       const SampleDistribution &sample_dist) {
    auto scene = view->scene_vk();

    auto ls_to_ws = xform.matrix_no_scale();
    auto ws_to_ls = glm::inverse(ls_to_ws);
    auto scene_aabb_ls =
        math::transform_aabb(ws_to_ls, scene->loaded_aabb_ws());

    auto frustum_vs = view->camera().frustum(view->size().w_div_h());
    auto frustum_ws =
        transform_frustum(view->xform().matrix_no_scale(), frustum_vs);
    auto cam_z_near = view->camera().z_near();
    auto cam_z_far = view->camera().z_far();

    // Log partition Z
    float z_partition_scale =
        std::pow(sample_dist.z_far / sample_dist.z_near,
                 1.0f / static_cast<float>(SHADOW_CASCADE_COUNT));

    auto sm_atlas_divide =
        static_cast<uint32_t>(std::ceil(std::sqrt(SHADOW_CASCADE_COUNT)));
    auto sm_atlas_size = _texture->info().extent.width;
    auto sm_atlas_grid_size =
        static_cast<uint32_t>(std::floor(sm_atlas_size / sm_atlas_divide));

    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        auto partition_z_near =
            sample_dist.z_near *
            std::pow(z_partition_scale, static_cast<float>(i));
        auto partition_z_far =
            sample_dist.z_near *
            std::pow(z_partition_scale, static_cast<float>(i) + 1.0f);

        auto effective_frustum_ws = frustum_ws.crop({
            {0.0f,
             0.0f,
             math::inverse_lerp(cam_z_near, cam_z_far, partition_z_near)},
            {1.0f,
             1.0f,
             math::inverse_lerp(cam_z_near, cam_z_far, partition_z_far)},
        });
        auto effective_frustum_ls =
            transform_frustum(ws_to_ls, effective_frustum_ws);

        auto visible_aabb_ls = effective_frustum_ls.aabb();
        auto visible_center_ls = visible_aabb_ls.center();
        auto visible_extent_ls = visible_aabb_ls.extent();

        // Make near plane fits scene aabb
        auto z_near = 0.01f;
        auto light_z = scene_aabb_ls.max.z + z_near;
        // Make far plane contains visible aabb
        auto z_far = light_z - visible_aabb_ls.min.z;

        auto light_center_ls =
            glm::vec3(visible_center_ls.x, visible_center_ls.y, light_z);
        auto light_center_ws =
            math::transform_position(ls_to_ws, light_center_ls);

        auto &cam = _cascade_cameras[i];
        // Update fields
        cam.xform = xform;
        cam.xform.set_translation(light_center_ws);

        cam.camera.z_near = z_near;
        cam.camera.z_far = z_far;
        cam.camera.y_mag =
            std::max(visible_extent_ls.x, visible_extent_ls.y) * 0.5f;

        auto sm_atlas_x = i % sm_atlas_divide;
        auto sm_atlas_y = i / sm_atlas_divide;
        auto grid_vp_size = static_cast<float>(sm_atlas_grid_size) /
                            static_cast<float>(sm_atlas_size);

        cam.viewport.x = static_cast<float>(sm_atlas_x) * grid_vp_size;
        cam.viewport.y = static_cast<float>(sm_atlas_y) * grid_vp_size;
        cam.viewport.w = grid_vp_size;
        cam.viewport.h = grid_vp_size;

        cam.partition_z_near = partition_z_near;
        cam.partition_z_far = partition_z_far;
    }
}

Handle<Texture> ShadowMap::texture() const {
    return _texture;
}

ShadowData ShadowMap::data(View *view) const {
    ShadowData d{};
    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        auto &cam = _cascade_cameras[i];
        auto &cascade = d.cascades[i];

        auto w_div_h = cam.viewport.w / cam.viewport.h;
        auto offset = glm::vec2(cam.viewport.x, cam.viewport.y);
        auto scale = glm::vec2(cam.viewport.w, cam.viewport.h);
        auto atlas_matrix =
            glm::translate(glm::identity<glm::mat4>(),
                           {scale + 2.0f * offset - 1.0f, 0.0f}) *
            glm::scale(glm::identity<glm::mat4>(), {scale, 1.0f});

        auto PS = cam.camera.projection_matrix(w_div_h);
        auto VS = glm::inverse(cam.xform.matrix_no_scale());
        auto I_V = glm::inverse(view->view_matrix());

        cascade.view_to_shadow_hclip = atlas_matrix * PS * VS * I_V;
        cascade.z_near = cam.partition_z_near;
        cascade.z_far = cam.partition_z_far;
    }

    return d;
}
} // namespace ars::render::vk