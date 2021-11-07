#include "QuerySelection.h"
#include "../Context.h"
#include "../Mesh.h"
#include "../Scene.h"
#include "../View.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
constexpr VkFormat QUERY_SELECTION_COLOR_ATTACHMENT_FORMAT =
    VK_FORMAT_R8G8B8A8_UNORM;

constexpr VkFormat QUERY_SELECTION_DEPTH_STENCIL_ATTACHMENT_FORMAT =
    VK_FORMAT_D32_SFLOAT;

namespace {
struct PushConstant {
    glm::mat4 MVP;
    glm::vec4 color_id;
};

glm::vec4 encode_color_id(uint32_t id) {
    auto encode_byte = [](uint32_t b) {
        return static_cast<float>(b) / 255.0f;
    };
    return {
        encode_byte(id & 255),
        encode_byte((id >> 8) & 255),
        encode_byte((id >> 16) & 255),
        encode_byte((id >> 24) & 255),
    };
}

uint32_t decode_color_id(uint32_t r, uint32_t g, uint32_t b, uint32_t a) {
    return r | (g << 8) | (b << 16) | (a << 24);
}
} // namespace

QuerySelection::QuerySelection(View *view) : _view(view) {
    init_render_pass();
    init_pipeline();
}

std::vector<uint64_t> QuerySelection::query_selection(uint32_t x,
                                                      uint32_t y,
                                                      uint32_t width,
                                                      uint32_t height) {
    auto ctx = _view->context();

    auto color_attach_info = TextureCreateInfo::sampled_2d(
        QUERY_SELECTION_COLOR_ATTACHMENT_FORMAT, width, height, 1);
    color_attach_info.usage =
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    auto depth_attach_info =
        TextureCreateInfo::sampled_2d(VK_FORMAT_D32_SFLOAT, width, height, 1);
    depth_attach_info.aspect_mask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_attach_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    auto color_attach = ctx->create_texture(color_attach_info);
    auto depth_attach = ctx->create_texture(depth_attach_info);

    auto fb = ctx->create_tmp_framebuffer(_render_pass.get(),
                                          {color_attach, depth_attach});

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

    // Synchronization can be optimized
    queue->submit_once([&](CommandBuffer *cmd) {
        // Clear all rts to zero
        VkClearValue clear_values[5]{};
        auto rp_exec = _render_pass->begin(
            cmd, fb, clear_values, VK_SUBPASS_CONTENTS_INLINE);

        _pipeline->bind(cmd);
        fb->set_viewport_scissor(cmd);

        auto v_matrix = _view->view_matrix();
        auto w_div_h = static_cast<float>(view_extent.width) /
                       static_cast<float>(view_extent.height);
        auto p_matrix =
            projection_scissor_correction(
                x, y, width, height, view_extent.width, view_extent.height) *
            _view->camera().projection_matrix(w_div_h);

        auto matrix_arr = rd_objs.get_array<glm::mat4>();
        auto mesh_arr = rd_objs.get_array<std::shared_ptr<Mesh>>();
        for (int i = 0; i < rd_obj_count; i++) {
            auto &matrix = matrix_arr[i];
            auto &mesh = mesh_arr[i];
            if (mesh == nullptr) {
                return;
            }

            VkBuffer vertex_buffers[] = {
                mesh->position_buffer()->buffer(),
            };
            VkDeviceSize vertex_offsets[std::size(vertex_buffers)] = {};

            PushConstant push_constant{};
            push_constant.MVP = p_matrix * v_matrix * matrix;
            // None zero id
            push_constant.color_id = encode_color_id(i + 1);

            cmd->PushConstants(_pipeline->pipeline_layout(),
                               VK_SHADER_STAGE_VERTEX_BIT |
                                   VK_SHADER_STAGE_FRAGMENT_BIT,
                               0,
                               sizeof(PushConstant),
                               &push_constant);
            cmd->BindVertexBuffers(
                0,
                static_cast<uint32_t>(std::size(vertex_buffers)),
                vertex_buffers,
                vertex_offsets);
            cmd->BindIndexBuffer(
                mesh->index_buffer()->buffer(), 0, VK_INDEX_TYPE_UINT32);
            cmd->DrawIndexed(mesh->triangle_count() * 3, 1, 0, 0, 0);
        }

        _render_pass->end(rp_exec);

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

    auto user_data_arr = rd_objs.get_array<RenderObjectUserData>();

    result_buffer->map_once([&](void *ptr) {
        auto byte_ptr = reinterpret_cast<uint8_t *>(ptr);
        for (int i = 0; i < width * height; i++) {
            auto index = i * 4;
            auto r = byte_ptr[index];
            auto g = byte_ptr[index + 1];
            auto b = byte_ptr[index + 2];
            auto a = byte_ptr[index + 3];
            auto id = decode_color_id(r, g, b, a);

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

void QuerySelection::init_render_pass() {
    RenderPassAttachmentInfo color_info{};
    color_info.format = QUERY_SELECTION_COLOR_ATTACHMENT_FORMAT;
    color_info.samples = VK_SAMPLE_COUNT_1_BIT;

    RenderPassAttachmentInfo depth_info{};
    depth_info.format = QUERY_SELECTION_DEPTH_STENCIL_ATTACHMENT_FORMAT;
    depth_info.samples = VK_SAMPLE_COUNT_1_BIT;

    _render_pass = RenderPass::create_with_single_pass(
        _view->context(), 1, &color_info, &depth_info);
}

void QuerySelection::init_pipeline() {
    auto ctx = _view->context();
    auto vert_shader = std::make_unique<Shader>(ctx, "ObjectId.vert");
    auto frag_shader = std::make_unique<Shader>(ctx, "ObjectId.frag");

    VkPipelineVertexInputStateCreateInfo vertex_input{
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

    VkVertexInputBindingDescription vert_bindings[1] = {
        {0,
         static_cast<uint32_t>(sizeof(glm::vec3)),
         VK_VERTEX_INPUT_RATE_VERTEX},
    };

    VkVertexInputAttributeDescription vert_attrs[1] = {
        {0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0},
    };

    vertex_input.vertexAttributeDescriptionCount =
        static_cast<uint32_t>(std::size(vert_attrs));
    vertex_input.pVertexAttributeDescriptions = vert_attrs;
    vertex_input.vertexBindingDescriptionCount =
        static_cast<uint32_t>(std::size(vert_bindings));
    vertex_input.pVertexBindingDescriptions = vert_bindings;

    auto depth_stencil = enabled_depth_stencil_state();

    VkPushConstantRange push_constant_range = {VK_SHADER_STAGE_VERTEX_BIT |
                                                   VK_SHADER_STAGE_FRAGMENT_BIT,
                                               0,
                                               sizeof(PushConstant)};

    GraphicsPipelineInfo info{};
    info.shaders.push_back(vert_shader.get());
    info.shaders.push_back(frag_shader.get());
    info.render_pass = _render_pass.get();
    info.subpass = 0;

    info.vertex_input = &vertex_input;
    info.depth_stencil = &depth_stencil;

    info.push_constant_range_count = 1;
    info.push_constant_ranges = &push_constant_range;

    _pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
}
} // namespace ars::render::vk