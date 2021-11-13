#include "QuerySelection.h"
#include "../Context.h"
#include "../Mesh.h"
#include "../Scene.h"
#include "../View.h"
#include "Drawer.h"

namespace ars::render::vk {
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

    auto view_extent = _view->size();
    // Constrain input range
    x = std::clamp(x, 0u, view_extent.width);
    y = std::clamp(y, 0u, view_extent.height);
    width = std::clamp(x + width, 0u, view_extent.width) - x;
    height = std::clamp(y + height, 0u, view_extent.height) - y;

    std::vector<glm::mat4> mvp_arr{};
    std::vector<uint32_t> ids{};
    mvp_arr.reserve(rd_obj_count);
    ids.reserve(rd_obj_count);

    auto v_matrix = _view->view_matrix();
    auto w_div_h = static_cast<float>(view_extent.width) /
                   static_cast<float>(view_extent.height);
    auto p_matrix =
        projection_scissor_correction(
            x, y, width, height, view_extent.width, view_extent.height) *
        _view->camera().projection_matrix(w_div_h);

    auto matrix_arr = rd_objs.get_array<glm::mat4>();
    for (int i = 0; i < rd_obj_count; i++) {
        mvp_arr.push_back(p_matrix * v_matrix * matrix_arr[i]);
        // None zero id
        ids.push_back(i + 1);
    }

    // Synchronization can be optimized
    queue->submit_once([&](CommandBuffer *cmd) {
        _view->drawer()->draw_ids(cmd,
                                  color_attach,
                                  depth_attach,
                                  rd_obj_count,
                                  mvp_arr.data(),
                                  ids.data(),
                                  rd_objs.get_array<std::shared_ptr<Mesh>>());

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

    auto user_data_arr = rd_objs.get_array<UserData>();

    result_buffer->map_once([&](void *ptr) {
        auto id_ptr = reinterpret_cast<uint32_t *>(ptr);
        for (int i = 0; i < width * height; i++) {
            auto id = id_ptr[i];
            if (id == 0 || id > rd_obj_count) {
                // no object found or invalid id
                continue;
            }
            result_set.insert(user_data_arr[id - 1].value);
        }
    });

    std::vector<uint64_t> result(result_set.begin(), result_set.end());
    return result;
}
} // namespace ars::render::vk