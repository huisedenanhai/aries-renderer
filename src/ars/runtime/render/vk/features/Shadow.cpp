#include "Shadow.h"
#include "../Context.h"
#include "../Profiler.h"
#include "Drawer.h"
#include <future>

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
            auto block_extent = 1u << _hiz_buffer_capture_level;
            _hiz_block_uv_size.x = static_cast<float>(block_extent) /
                                   static_cast<float>(hiz_info.extent.width);
            _hiz_block_uv_size.y = static_cast<float>(block_extent) /
                                   static_cast<float>(hiz_info.extent.height);

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

namespace {
void calculate_hiz_depth01_range(glm::vec2 *hiz_pixels,
                                 uint32_t hiz_buffer_width,
                                 uint32_t hiz_buffer_height,
                                 float &min_depth01,
                                 float &max_depth01) {
    min_depth01 = 1.0f;
    max_depth01 = 0.0f;
    for (int y = 0; y < hiz_buffer_height; y++) {
        for (int x = 0; x < hiz_buffer_width; x++) {
            auto i = y * hiz_buffer_width + x;
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
}

// Return (min_x, min_y, max_x, max_y)
glm::vec4 calculate_valid_viewport(glm::vec2 *hiz_pixels,
                                   uint32_t hiz_buffer_width,
                                   uint32_t hiz_buffer_height,
                                   glm::vec2 hiz_block_uv_size,
                                   float min_depth01,
                                   float max_depth01) {
    float min_x = 1.0f, max_x = 0.0f;
    float min_y = 1.0f, max_y = 0.0f;
    for (int y = 0; y < hiz_buffer_height; y++) {
        for (int x = 0; x < hiz_buffer_width; x++) {
            auto i = y * hiz_buffer_width + x;
            auto depth01 = hiz_pixels[i];
            assert(depth01.x >= depth01.y);
            // Check if there is any sample is inside the depth range
            if (depth01.x < min_depth01 || depth01.y > max_depth01) {
                continue;
            }
            auto block_min_x = static_cast<float>(x) * hiz_block_uv_size.x;
            auto block_max_x = static_cast<float>(x + 1) * hiz_block_uv_size.x;
            auto block_min_y = static_cast<float>(y) * hiz_block_uv_size.y;
            auto block_max_y = static_cast<float>(y + 1) * hiz_block_uv_size.y;

            min_x = std::min(block_min_x, min_x);
            max_x = std::max(block_max_x, max_x);
            min_y = std::min(block_min_y, min_y);
            max_y = std::max(block_max_y, max_y);
        }
    }

    // Empty viewport
    if (min_x > max_x || min_y > max_y) {
        min_x = max_x = min_y = max_y = 0.0f;
    }

    // Clamp in range, block resolution is of power of 2.
    min_x = std::max(0.0f, min_x);
    max_x = std::min(1.0f, max_x);
    min_y = std::max(0.0f, min_y);
    max_y = std::min(1.0f, max_y);

    return {min_x, min_y, max_x, max_y};
}
} // namespace

SampleDistribution Shadow::calculate_sample_distribution() {
    ARS_PROFILER_SAMPLE("Calculate Depth Sample Dist", 0xFF999411);
    if (_last_frame_hiz_data == nullptr) {
        return {};
    }

    auto hiz_pixels =
        reinterpret_cast<glm::vec2 *>(_last_frame_hiz_data->map());
    ARS_DEFER([&]() { _last_frame_hiz_data->unmap(); });

    float min_depth01, max_depth01;
    calculate_hiz_depth01_range(hiz_pixels,
                                _hiz_buffer_width,
                                _hiz_buffer_height,
                                min_depth01,
                                max_depth01);

    SampleDistribution dist{};

    auto last_frame_view = _view->last_frame_data();
    auto last_frame_P = last_frame_view.projection_matrix();
    auto last_frame_I_P = glm::inverse(last_frame_P);
    auto last_frame_frustum_ws = last_frame_view.frustum_ws();
    auto cam_z_near = last_frame_view.camera.z_near();
    auto cam_z_far = last_frame_view.camera.z_far();

    dist.z_near = depth01_to_linear_z(last_frame_I_P, max_depth01);
    dist.z_far = depth01_to_linear_z(last_frame_I_P, min_depth01);

    // Log partition Z
    float z_partition_scale =
        std::pow(dist.z_far / dist.z_near,
                 1.0f / static_cast<float>(SHADOW_CASCADE_COUNT));

    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        auto get_partition_z = [&](float r) {
            return dist.z_near * std::pow(z_partition_scale, r);
        };

        // The padding used for frustum partition
        auto padding = 0.1f;
        auto partition_z_near =
            get_partition_z(static_cast<float>(i) - padding);
        auto partition_z_far =
            get_partition_z(static_cast<float>(i) + 1.0f + padding);

        // The padding used for cascade selection. The transition padding is
        // smaller than partition padding, so the reprojected z may have more
        // chance to be valid.
        auto transition_padding = 0.05f;
        auto transition_z_near =
            get_partition_z(static_cast<float>(i) - transition_padding);
        auto transition_z_far =
            get_partition_z(static_cast<float>(i) + 1.0f + transition_padding);

        auto viewport = calculate_valid_viewport(
            hiz_pixels,
            _hiz_buffer_width,
            _hiz_buffer_height,
            _hiz_block_uv_size,
            linear_z_to_depth01(last_frame_P, partition_z_far),
            linear_z_to_depth01(last_frame_P, partition_z_near));

        auto viewport_padding = 0.5f * _hiz_block_uv_size;

        auto effective_frustum_ws = last_frame_frustum_ws.crop({
            {viewport.x - viewport_padding.x,
             viewport.y - viewport_padding.y,
             math::inverse_lerp(cam_z_near, cam_z_far, partition_z_near)},
            {viewport.z + viewport_padding.x,
             viewport.w + viewport_padding.y,
             math::inverse_lerp(cam_z_near, cam_z_far, partition_z_far)},
        });

        auto reproject_IV_V =
            _view->view_matrix() * glm::inverse(last_frame_view.view_matrix());

        auto reproject_z = [&](float last_frame_z) {
            auto pos =
                reproject_IV_V * glm::vec4(0.0f, 0.0f, -last_frame_z, 1.0f);
            return -pos.z / pos.w;
        };

        dist.cascades[i].frustum_ws = effective_frustum_ws;
        dist.cascades[i].transition_z_near = reproject_z(transition_z_near);
        dist.cascades[i].transition_z_far = reproject_z(transition_z_far);
    }

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

            std::future<std::vector<DrawRequest>>
                shadow_draw_requests[SHADOW_CASCADE_COUNT]{};

            {
                ARS_PROFILER_SAMPLE("Shadow Culling", 0xFF771128);
                for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
                    shadow_draw_requests[i] =
                        std::async(std::launch::async, [i, this, scene]() {
                            auto &cam = _cascade_cameras[i];
                            float w_div_h = cam.viewport.w / cam.viewport.h;
                            auto shadow_cull_obj = scene->cull(
                                cam.xform, cam.camera.frustum(w_div_h));

                            return shadow_cull_obj.gather_draw_requests(
                                RenderPassID_Shadow);
                        });
                }

                for (auto &fut : shadow_draw_requests) {
                    fut.wait();
                }
            }

            // Render each cascades
            for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
                auto &cam = _cascade_cameras[i];
                fb->set_viewport_scissor(cmd,
                                         cam.viewport.x,
                                         cam.viewport.y,
                                         cam.viewport.w,
                                         cam.viewport.h);

                DrawCallbacks callbacks{};
                callbacks.on_pipeline_bound = [&](CommandBuffer *cmd) {
                    // Negative sign to bias because we use inverse z
                    cmd->SetDepthBias(-_constant_bias, 0.0f, -_slope_bias);
                };

                float w_div_h = cam.viewport.w / cam.viewport.h;
                view->drawer()->draw(cmd,
                                     cam.camera.projection_matrix(w_div_h),
                                     glm::inverse(cam.xform.matrix_no_scale()),
                                     shadow_draw_requests[i].get(),
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

    auto sm_atlas_divide =
        static_cast<uint32_t>(std::ceil(std::sqrt(SHADOW_CASCADE_COUNT)));
    auto sm_atlas_size = _texture->info().extent.width;
    auto sm_atlas_grid_size =
        static_cast<uint32_t>(std::floor(sm_atlas_size / sm_atlas_divide));

    for (int i = 0; i < SHADOW_CASCADE_COUNT; i++) {
        auto &cascade = sample_dist.cascades[i];
        auto effective_frustum_ls =
            transform_frustum(ws_to_ls, cascade.frustum_ws);

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

        cam.transition_z_near = cascade.transition_z_near;
        cam.transition_z_far = cascade.transition_z_far;
    }
}

Handle<Texture> ShadowMap::texture() const {
    return _texture;
}

ShadowData ShadowMap::data(View *view) const {
    ShadowData d{};
    auto sm_pixel_size =
        1.0f / static_cast<float>(_texture->info().extent.width);

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
        cascade.z_near = cam.transition_z_near;
        cascade.z_far = cam.transition_z_far;

        cascade.uv_clamp = {
            cam.viewport.x + sm_pixel_size,
            cam.viewport.y + sm_pixel_size,
            cam.viewport.x + cam.viewport.w - sm_pixel_size,
            cam.viewport.y + cam.viewport.h - sm_pixel_size,
        };
    }

    return d;
}
} // namespace ars::render::vk