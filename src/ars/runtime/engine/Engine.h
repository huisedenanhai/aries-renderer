#pragma once

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

    [[nodiscard]] virtual std::string get_name() const;

    virtual void start() {}
    virtual void update() {}
    virtual void on_imgui() {}
    virtual void destroy() {}
};

// Engine is the runtime, the singleton that manages the life-time of all
// modules and interactions between them.
//
// DO NOT put these getter methods into a single class, as all methods of a
// class should be defined in the same header, effectively make this file a god
// header that frequently triggers full rebuild and slow down further iteration.
void start_engine(std::unique_ptr<IApplication> app);

render::IWindow *main_window();
render::IContext *render_context();
} // namespace engine
} // namespace ars