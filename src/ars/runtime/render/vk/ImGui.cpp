#include "ImGui.h"
#include "Context.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include <cassert>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>

namespace ars::render::vk {

namespace {
class ImGuiRendererData {
  public:
    explicit ImGuiRendererData(Swapchain *swapchain, ImGuiIO &io)
        : _swapchain(swapchain) {
        _reserved_space = std::make_unique<uint8_t[]>(16);
        init_font_texture(io);
        init_pipeline();
    }

    [[nodiscard]] ImTextureID font_atlas_id() const {
        return reinterpret_cast<ImTextureID>(_reserved_space.get());
    }

    [[nodiscard]] Texture *font_atlas() const {
        return _font_atlas.get();
    }

    [[nodiscard]] GraphicsPipeline *pipeline() const {
        return _pipeline.get();
    }

  private:
    void init_font_texture(ImGuiIO &io) {
        unsigned char *pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);
        auto data_size = width * height * 4 * sizeof(unsigned char);

        auto info = TextureCreateInfo::sampled_2d(
            VK_FORMAT_R8G8B8A8_UNORM, width, height, 1);

        _font_atlas = _swapchain->context()->create_texture(info);
        _font_atlas->set_data(
            pixels, data_size, 0, 0, 0, 0, 0, width, height, 1);

        io.Fonts->SetTexID(font_atlas_id());
    }

    void init_pipeline() {
        auto ctx = _swapchain->context();
        auto vert_shader = std::make_unique<Shader>(ctx, "ImGui.vert");
        auto frag_shader = std::make_unique<Shader>(ctx, "ImGui.frag");

        VkPipelineVertexInputStateCreateInfo vertex_input{
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};

        VkVertexInputBindingDescription vert_binding = {
            0,
            static_cast<uint32_t>(sizeof(ImDrawVert)),
            VK_VERTEX_INPUT_RATE_VERTEX};

        VkVertexInputAttributeDescription vert_attrs[3] = {
            {0, 0, VK_FORMAT_R32G32_SFLOAT, 0},
            {1,
             0,
             VK_FORMAT_R32G32_SFLOAT,
             static_cast<uint32_t>(offsetof(ImDrawVert, uv))},
            {2,
             0,
             VK_FORMAT_R8G8B8A8_UNORM,
             static_cast<uint32_t>(offsetof(ImDrawVert, col))},
        };

        vertex_input.vertexAttributeDescriptionCount =
            static_cast<uint32_t>(std::size(vert_attrs));
        vertex_input.pVertexAttributeDescriptions = vert_attrs;

        vertex_input.vertexBindingDescriptionCount = 1;
        vertex_input.pVertexBindingDescriptions = &vert_binding;

        VkPushConstantRange push_constant_range{
            VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(float) * 4};

        VkPipelineColorBlendStateCreateInfo blend{
            VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};

        VkPipelineColorBlendAttachmentState attachment =
            create_attachment_blend_state(VK_BLEND_FACTOR_SRC_ALPHA,
                                          VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA);

        blend.attachmentCount = 1;
        blend.pAttachments = &attachment;

        GraphicsPipelineInfo info{};
        info.shaders.push_back(vert_shader.get());
        info.shaders.push_back(frag_shader.get());
        info.render_pass = _swapchain->render_pass();
        info.subpass = 0;

        info.push_constant_range_count = 1;
        info.push_constant_ranges = &push_constant_range;

        info.vertex_input = &vertex_input;
        info.blend = &blend;

        _pipeline = std::make_unique<GraphicsPipeline>(ctx, info);
    }

    Swapchain *_swapchain{};
    // Allocate a small slice of memory addresses as reserved ImTextureID
    // By default ImTextureID should be ITexture *, but some built in textures
    // should have type Texture *.
    std::unique_ptr<uint8_t[]> _reserved_space{};
    Handle<Texture> _font_atlas{};
    std::unique_ptr<GraphicsPipeline> _pipeline{};
};

class ImGuiViewportData {
  public:
    explicit ImGuiViewportData(Swapchain *swapchain) : _swapchain(swapchain) {}

    Buffer *vertex_buffer(VkDeviceSize size = 0) {
        return create_or_resize(
            _vertex_buffer, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    }

    Buffer *index_buffer(VkDeviceSize size = 0) {
        return create_or_resize(
            _index_buffer, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    }

  private:
    Buffer *create_or_resize(Handle<Buffer> &buffer,
                             VkDeviceSize size,
                             VkBufferUsageFlags usage) {
        if (buffer.get() == nullptr || buffer->size() < size) {
            buffer = _swapchain->context()->create_buffer(
                std::max(size, 256ULL), usage, VMA_MEMORY_USAGE_CPU_TO_GPU);
        }

        return buffer.get();
    }

    Swapchain *_swapchain = nullptr;
    Handle<Buffer> _vertex_buffer{};
    Handle<Buffer> _index_buffer{};
};
} // namespace

ImGuiPass::ImGuiPass(Swapchain *swapchain) : _swapchain(swapchain) {
    assert(swapchain != nullptr);

    _imgui_context = ImGui::CreateContext();
    // Only the first context will reset global context after creation.
    make_current();
    auto &io = ImGui::GetIO();

    ImGui_ImplGlfw_InitForVulkan(_swapchain->window(), true);

    assert(io.BackendRendererUserData == nullptr &&
           "Already initialized a renderer backend!");

    // Setup backend capabilities flags
    auto bd = new ImGuiRendererData(swapchain, io);
    io.BackendRendererUserData = (void *)bd;
    io.BackendRendererName = "imgui_ars_vulkan";
    io.BackendFlags |=
        ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the
                                                // ImDrawCmd::VtxOffset field,
                                                // allowing for large meshes.
    io.BackendFlags |=
        ImGuiBackendFlags_RendererHasViewports; // We can create multi-viewports
                                                // on the Renderer side
                                                // (optional)

    ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    main_viewport->RendererUserData = new ImGuiViewportData(swapchain);

    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
        ImGuiPlatformIO &platform_io = ImGui::GetPlatformIO();
        platform_io.Renderer_CreateWindow = [](ImGuiViewport *viewport) {
            // TODO
        };
        platform_io.Renderer_DestroyWindow = [](ImGuiViewport *viewport) {
            // TODO
        };
        platform_io.Renderer_SetWindowSize = [](ImGuiViewport *viewport,
                                                ImVec2 size) {
            // TODO
        };
        platform_io.Renderer_RenderWindow = [](ImGuiViewport *viewport,
                                               void *args) {
            // TODO
        };
        platform_io.Renderer_SwapBuffers = [](ImGuiViewport *viewport,
                                              void *args) {
            // TODO
        };
    }
}

ImGuiPass::~ImGuiPass() {
    // Not initialized
    if (_imgui_context == nullptr) {
        return;
    }

    make_current();

    // Manually delete main viewport render data in-case we haven't initialized
    // for viewports
    ImGuiViewport *main_viewport = ImGui::GetMainViewport();
    auto vd =
        reinterpret_cast<ImGuiViewportData *>(main_viewport->RendererUserData);
    delete vd;
    main_viewport->RendererUserData = nullptr;

    // Close all platform windows
    ImGui::DestroyPlatformWindows();

    auto &io = ImGui::GetIO();
    io.BackendRendererName = nullptr;
    auto bd = reinterpret_cast<ImGuiRendererData *>(io.BackendRendererUserData);
    delete bd;
    io.BackendRendererUserData = nullptr;

    ImGui_ImplGlfw_Shutdown();

    ImGui::DestroyContext(_imgui_context);
}

void ImGuiPass::new_frame() {
    make_current();

    assert(ImGui::GetIO().BackendRendererUserData != nullptr &&
           "Renderer not initialized");

    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void ImGuiPass::draw(CommandBuffer *cmd) {
    make_current();

    ImGui::Render();
    ImDrawData *draw_data = ImGui::GetDrawData();

    auto vp_data = reinterpret_cast<ImGuiViewportData *>(
        draw_data->OwnerViewport->RendererUserData);

    assert(vp_data != nullptr);

    if (draw_data->TotalVtxCount > 0) {
        size_t vertex_size = draw_data->TotalVtxCount * sizeof(ImDrawVert);
        size_t index_size = draw_data->TotalIdxCount * sizeof(ImDrawIdx);

        auto vertex_buffer = vp_data->vertex_buffer(vertex_size);
        auto index_buffer = vp_data->index_buffer(index_size);

        auto vtx_dst = reinterpret_cast<ImDrawVert *>(vertex_buffer->map());
        auto idx_dst = reinterpret_cast<ImDrawIdx *>(index_buffer->map());

        for (int n = 0; n < draw_data->CmdListsCount; n++) {
            const ImDrawList *cmd_list = draw_data->CmdLists[n];
            memcpy(vtx_dst,
                   cmd_list->VtxBuffer.Data,
                   cmd_list->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(idx_dst,
                   cmd_list->IdxBuffer.Data,
                   cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx));
            vtx_dst += cmd_list->VtxBuffer.Size;
            idx_dst += cmd_list->IdxBuffer.Size;
        }

        vertex_buffer->unmap();
        index_buffer->unmap();
    }

    auto fb_extent = _swapchain->physical_size();
    auto fb_width = static_cast<float>(fb_extent.width);
    auto fb_height = static_cast<float>(fb_extent.height);

    auto setup_render_state = [&]() {
        auto bd = reinterpret_cast<ImGuiRendererData *>(
            ImGui::GetIO().BackendRendererUserData);

        auto pipeline = bd->pipeline();

        // Bind pipeline and descriptor sets:
        {
            cmd->BindPipeline(VK_PIPELINE_BIND_POINT_GRAPHICS,
                              pipeline->pipeline());

            auto desc_set = pipeline->alloc_desc_set(0);

            VkWriteDescriptorSet write{};
            VkDescriptorImageInfo image_info{};
            fill_combined_image_sampler(
                &write, &image_info, desc_set, 0, bd->font_atlas());

            _swapchain->context()->device()->UpdateDescriptorSets(
                1, &write, 0, nullptr);

            cmd->BindDescriptorSets(VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    pipeline->pipeline_layout(),
                                    0,
                                    1,
                                    &desc_set,
                                    0,
                                    nullptr);
        }

        // Bind Vertex And Index Buffer:
        if (draw_data->TotalVtxCount > 0) {
            VkBuffer vertex_buffers[1] = {vp_data->vertex_buffer()->buffer()};
            VkDeviceSize vertex_offset[1] = {0};
            cmd->BindVertexBuffers(0, 1, vertex_buffers, vertex_offset);

            // There might be a warning for sizeof(ImDrawIdx) == 2 since the
            // linter thinks this expr is always true and redundant, which is
            // not, as the sizeof ImDrawIdx is configurable at compile time.
            cmd->BindIndexBuffer(vp_data->index_buffer()->buffer(),
                                 0,
                                 sizeof(ImDrawIdx) == 2 ? VK_INDEX_TYPE_UINT16
                                                        : VK_INDEX_TYPE_UINT32);
        }

        // Setup viewport:
        {
            VkViewport viewport;
            viewport.x = 0;
            viewport.y = 0;
            viewport.width = (float)fb_width;
            viewport.height = (float)fb_height;
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            cmd->SetViewport(0, 1, &viewport);
        }

        // Setup scale and translation:
        // Our visible imgui space lies from draw_data->DisplayPps (top left) to
        // draw_data->DisplayPos+data_data->DisplaySize (bottom right).
        // DisplayPos is (0,0) for single viewport apps.
        {
            float scale[2];
            scale[0] = 2.0f / draw_data->DisplaySize.x;
            scale[1] = 2.0f / draw_data->DisplaySize.y;
            float translate[2];
            translate[0] = -1.0f - draw_data->DisplayPos.x * scale[0];
            translate[1] = -1.0f - draw_data->DisplayPos.y * scale[1];
            cmd->PushConstants(pipeline->pipeline_layout(),
                               VK_SHADER_STAGE_VERTEX_BIT,
                               sizeof(float) * 0,
                               sizeof(float) * 2,
                               scale);
            cmd->PushConstants(pipeline->pipeline_layout(),
                               VK_SHADER_STAGE_VERTEX_BIT,
                               sizeof(float) * 2,
                               sizeof(float) * 2,
                               translate);
        }
    };

    setup_render_state();

    // Will project scissor/clipping rectangles into framebuffer space
    ImVec2 clip_off =
        draw_data->DisplayPos; // (0,0) unless using multi-viewports
    ImVec2 clip_scale =
        draw_data->FramebufferScale; // (1,1) unless using retina display which
                                     // are often (2,2)

    // Render command lists
    // (Because we merged all buffers into a single one, we maintain our own
    // offset into them)
    int global_vtx_offset = 0;
    int global_idx_offset = 0;
    for (int n = 0; n < draw_data->CmdListsCount; n++) {
        const ImDrawList *cmd_list = draw_data->CmdLists[n];
        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback != nullptr) {
                // User callback, registered via ImDrawList::AddCallback()
                // (ImDrawCallback_ResetRenderState is a special callback value
                // used by the user to request the renderer to reset render
                // state.)
                if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
                    setup_render_state();
                } else {
                    pcmd->UserCallback(cmd_list, pcmd);
                }
            } else {
                // Project scissor/clipping rectangles into framebuffer space
                ImVec2 clip_min((pcmd->ClipRect.x - clip_off.x) * clip_scale.x,
                                (pcmd->ClipRect.y - clip_off.y) * clip_scale.y);
                ImVec2 clip_max((pcmd->ClipRect.z - clip_off.x) * clip_scale.x,
                                (pcmd->ClipRect.w - clip_off.y) * clip_scale.y);

                // Clamp to viewport as vkCmdSetScissor() won't accept values
                // that are off bounds
                clip_min.x = std::clamp(clip_min.x, 0.0f, fb_width);
                clip_min.y = std::clamp(clip_min.y, 0.0f, fb_height);

                if (clip_max.x < clip_min.x || clip_max.y < clip_min.y) {
                    continue;
                }

                VkRect2D scissor;
                scissor.offset.x = (int32_t)(clip_min.x);
                scissor.offset.y = (int32_t)(clip_min.y);
                scissor.extent.width = (uint32_t)(clip_max.x - clip_min.x);
                scissor.extent.height = (uint32_t)(clip_max.y - clip_min.y);

                cmd->SetScissor(0, 1, &scissor);

                cmd->DrawIndexed(
                    pcmd->ElemCount,
                    1,
                    pcmd->IdxOffset + global_idx_offset,
                    static_cast<int32_t>(pcmd->VtxOffset + global_vtx_offset),
                    0);
            }
        }
        global_idx_offset += cmd_list->IdxBuffer.Size;
        global_vtx_offset += cmd_list->VtxBuffer.Size;
    }
}

void ImGuiPass::make_current() const {
    ImGui::SetCurrentContext(_imgui_context);
}
} // namespace ars::render::vk