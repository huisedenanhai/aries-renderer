#pragma once

#include <functional>
#include <memory>
#include <string>

namespace ars {
namespace render {
class IWindow;
class IContext;
} // namespace render

namespace engine {
class IApplication {
  public:
    virtual ~IApplication() = default;

    void init(render::IWindow *window);
    render::IWindow *window() const;

    void quit();
    bool want_to_quit() const;

    [[nodiscard]] virtual std::string get_name() const;

    // Callbacks to override
    virtual void start() {}
    virtual void update(double dt) {}
    virtual void on_imgui() {}
    virtual void destroy() {}

  private:
    render::IWindow *_window = nullptr;
    bool _want_to_quit = false;
};

// Engine is the runtime, the singleton that manages the life-time of all
// modules and interactions between them.
//
// DO NOT put these getter methods into a single class, as all methods of a
// class should be defined in the same header, effectively make this file a god
// header that frequently triggers full rebuild and slow down further iteration.
void start_engine(std::unique_ptr<IApplication> app);

using WindowRenderCallback = std::function<void(render::IWindow *, double dt)>;

void create_window(const char *title,
                   uint32_t logical_width,
                   uint32_t logical_height,
                   WindowRenderCallback render);

render::IContext *render_context();
} // namespace engine
} // namespace ars