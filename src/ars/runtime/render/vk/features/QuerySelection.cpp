#include "QuerySelection.h"
#include "../Context.h"
#include "../Mesh.h"
#include "../Scene.h"
#include "../View.h"
#include "Drawer.h"
#include "OverlayRenderer.h"

namespace ars::render::vk {
constexpr uint32_t ID_OBJECT_MARK = 0x01000000;
constexpr uint32_t ID_POINT_LIGHT_MARK = 0x02000000;
constexpr uint32_t ID_DIRECTIONAL_LIGHT_MARK = 0x03000000;
constexpr uint32_t ID_VALUE_MASK = 0x00FFFFFF;
constexpr uint32_t ID_MARK_MASK = 0xFF000000;

QuerySelection::QuerySelection(View *view) : _view(view) {}

std::vector<uint64_t> QuerySelection::query_selection(uint32_t x,
                                                      uint32_t y,
                                                      uint32_t width,
                                                      uint32_t height) {
    auto ctx = _view->context();

    auto color_attach_info = TextureCreateInfo::sampled_2d(
        ID_COLOR_ATTACHMENT_FORMAT, width, height, 1);
    color_attach_info.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    auto depth_attach_info = TextureCreateInfo::sampled_2d(
        ID_DEPTH_STENCIL_ATTACHMENT_FORMAT, width, height, 1);
    depth_attach_info.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_attach_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    auto color_attach = ctx->create_texture(color_attach_info);
    auto depth_attach = ctx->create_texture(depth_attach_info);

    auto queue = ctx->queue();

    auto result_buffer =
        std::make_unique<Buffer>(ctx,
                                 4 * width * height,
                                 VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                 VMA_MEMORY_USAGE_GPU_TO_CPU);

    auto &rd_objs = _view->vk_scene()->render_objects;
    auto rd_obj_count = rd_objs.size();
    auto &point_lights = _view->vk_scene()->point_lights;
    auto point_light_count = point_lights.size();
    auto &directional_lights = _view->vk_scene()->directional_lights;
    auto directional_light_count = directional_lights.size();
    auto light_count = point_light_count + directional_light_count;

    auto view_extent = _view->size();
    // Constrain input range
    x = std::clamp(x, 0u, view_extent.width);
    y = std::clamp(y, 0u, view_extent.height);
    width = std::clamp(x + width, 0u, view_extent.width) - x;
    height = std::clamp(y + height, 0u, view_extent.height) - y;

    auto v_matrix = _view->view_matrix();
    auto w_div_h = static_cast<float>(view_extent.width) /
                   static_cast<float>(view_extent.height);
    auto p_matrix =
        projection_scissor_correction(
            x, y, width, height, view_extent.width, view_extent.height) *
        _view->camera().projection_matrix(w_div_h);

    // Synchronization can be optimized
    queue->submit_once([&](CommandBuffer *cmd) {
        auto id_rp = _view->drawer()->draw_id_render_pass();
        auto exec = id_rp->begin(
            cmd, {color_attach, depth_attach}, VK_SUBPASS_CONTENTS_INLINE);
        // draw render object ids
        {
            std::vector<glm::mat4> mvp_arr{};
            std::vector<uint32_t> ids{};
            mvp_arr.reserve(rd_obj_count);
            ids.reserve(rd_obj_count);
            auto matrix_arr = rd_objs.get_array<glm::mat4>();
            for (int i = 0; i < rd_obj_count; i++) {
                mvp_arr.push_back(p_matrix * v_matrix * matrix_arr[i]);
                ids.push_back(static_cast<uint32_t>(i) | ID_OBJECT_MARK);
            }

            _view->drawer()->draw_ids(
                cmd,
                rd_obj_count,
                mvp_arr.data(),
                ids.data(),
                rd_objs.get_array<std::shared_ptr<Mesh>>());
        }

        auto light_gizmo = _view->vk_overlay()->light_gizmo();
        if (light_gizmo.texture != nullptr) {
            std::vector<glm::mat4> mvp_arr{};
            std::vector<uint32_t> ids{};
            std::vector<Handle<Texture>> textures{};
            mvp_arr.reserve(light_count);
            ids.reserve(light_count);
            textures.reserve(light_count);

            auto tex_extent = light_gizmo.texture->info().extent;
            auto width = light_gizmo.width;
            auto height = width * static_cast<float>(tex_extent.height) /
                          static_cast<float>(tex_extent.width);

            auto prepare = [&](auto &&lights, uint32_t mark) {
                auto xform_arr =
                    lights.template get_array<math::XformTRS<float>>();
                for (int i = 0; i < lights.size(); i++) {
                    mvp_arr.push_back(p_matrix * _view->billboard_MV_matrix(
                                                     xform_arr[i].translation(),
                                                     width,
                                                     height));
                    ids.push_back(mark | static_cast<uint32_t>(i));
                    textures.push_back(light_gizmo.texture);
                }
            };
            prepare(point_lights, ID_POINT_LIGHT_MARK);
            prepare(directional_lights, ID_DIRECTIONAL_LIGHT_MARK);

            _view->drawer()->draw_ids_billboard_alpha_clip(
                cmd,
                static_cast<uint32_t>(mvp_arr.size()),
                mvp_arr.data(),
                ids.data(),
                textures.data());
        }

        id_rp->end(exec);

        VkImageMemoryBarrier barrier{VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
        barrier.image = color_attach->image();
        barrier.subresourceRange = color_attach->subresource_range();
        barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.oldLayout = color_attach->layout();
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;

        cmd->PipelineBarrier(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                             VK_PIPELINE_STAGE_TRANSFER_BIT,
                             0,
                             0,
                             nullptr,
                             0,
                             nullptr,
                             1,
                             &barrier);

        color_attach->assure_layout(VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

        // offset 0, tightly packed
        VkBufferImageCopy region{};
        region.imageExtent = color_attach->info().extent;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageSubresource.mipLevel = 0;

        cmd->CopyImageToBuffer(color_attach->image(),
                               color_attach->layout(),
                               result_buffer->buffer(),
                               1,
                               &region);
    });

    queue->flush();

    std::set<uint64_t> result_set{};

    result_buffer->map_once([&](void *ptr) {
        auto id_ptr = reinterpret_cast<uint32_t *>(ptr);
        for (int i = 0; i < width * height; i++) {
            auto id = id_ptr[i] & ID_VALUE_MASK;
            auto id_mark = id_ptr[i] & ID_MARK_MASK;
            if (id_mark == ID_OBJECT_MARK && id < rd_obj_count) {
                result_set.insert(rd_objs.get_array<UserData>()[id].value);
            }
            if (id_mark == ID_POINT_LIGHT_MARK && id < point_light_count) {
                result_set.insert(point_lights.get_array<UserData>()[id].value);
            }
            if (id_mark == ID_DIRECTIONAL_LIGHT_MARK &&
                id < directional_light_count) {
                result_set.insert(
                    directional_lights.get_array<UserData>()[id].value);
            }
        }
    });

    std::vector<uint64_t> result(result_set.begin(), result_set.end());
    return result;
}
} // namespace ars::render::vk