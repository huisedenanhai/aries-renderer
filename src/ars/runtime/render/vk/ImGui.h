#pragma once

#include <memory>

struct ImGuiContext;
struct ImGuiViewport;

namespace ars::render::vk {
class Swapchain;
class CommandBuffer;

class ImGuiPass {
  public:
    explicit ImGuiPass(Swapchain *swapchain);
    ~ImGuiPass();

    void new_frame();
    void end_frame();
    // The render pass should already begin on the cmd buffer
    // If the viewport = nullptr, use the main viewport
    void draw(CommandBuffer *cmd, ImGuiViewport *viewport = nullptr);

  private:
    void make_current() const;

    ImGuiContext *_imgui_context = nullptr;
};
} // namespace ars::render::vk
