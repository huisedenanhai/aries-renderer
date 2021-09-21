#include "Engine.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ITexture.h>
#include <ars/runtime/render/IWindow.h>
#include <cassert>

namespace ars::engine {
std::string engine::IApplication::get_name() const {
    return "";
}

namespace {
class Engine;

Engine *s_engine = nullptr;

class Engine {
  public:
    explicit Engine(std::unique_ptr<IApplication> app)
        : _application(std::move(app)) {}

    void run() {
        init_render();

        _application->start();

        while (!_window->should_close()) {
            if (_render_context->begin_frame()) {
                _application->update();
                _view->render();
                _window->present(_view->get_color_texture());
                _render_context->end_frame();
            }
        }

        _application->destroy();
        _application.reset();

        destroy_render();
    }

    [[nodiscard]] render::IWindow *window() const {
        return _window.get();
    }

    [[nodiscard]] render::IContext *render_context() const {
        return _render_context.get();
    }

  private:
    void init_render() {
        using namespace render;
        ApplicationInfo app_info{};
        app_info.app_name = _application->get_name();
        app_info.enable_validation = true;

        init_render_backend(app_info);

        WindowInfo win_info{};
        win_info.title = _application->get_name();
        auto [ctx, window] = IContext::create(&win_info);

        _render_context = std::move(ctx);
        _window = std::move(window);

        _window->set_imgui_callback([this]() { _application->on_imgui(); });

        _scene = _render_context->create_scene();
        _view = _scene->create_view();
    }

    void destroy_render() {
        using namespace render;

        _scene.reset();
        _view.reset();
        _window.reset();
        _render_context.reset();

        destroy_render_backend();
    }

    std::unique_ptr<IApplication> _application{};
    std::unique_ptr<render::IContext> _render_context{};
    std::unique_ptr<render::IWindow> _window{};
    std::shared_ptr<render::IScene> _scene{};
    std::shared_ptr<render::IView> _view{};
};

} // namespace

void start_engine(std::unique_ptr<IApplication> app) {
    assert(s_engine == nullptr);
    assert(app != nullptr && "An application call back should be provided");

    auto engine = std::make_unique<Engine>(std::move(app));
    s_engine = engine.get();

    engine->run();

    s_engine = nullptr;
    engine.reset();
}

render::IWindow *main_window() {
    assert(s_engine != nullptr);
    return s_engine->window();
}

render::IContext *render_context() {
    assert(s_engine != nullptr);
    return s_engine->render_context();
}
} // namespace ars::engine