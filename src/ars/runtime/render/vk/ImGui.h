#pragma once

#include <memory>

struct ImGuiContext;

namespace ars::render::vk {
class Swapchain;
class CommandBuffer;

class ImGuiPass {
  public:
    explicit ImGuiPass(Swapchain *swapchain);
    ~ImGuiPass();

    void new_frame();
    // The render pass should already begin on the cmd buffer
    void draw(CommandBuffer *cmd);

  private:
    void make_current() const;
    Swapchain *_swapchain = nullptr;
    ImGuiContext *_imgui_context = nullptr;
};
} // namespace ars::render::vk
