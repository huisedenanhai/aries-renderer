#include "Engine.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ITexture.h>
#include <ars/runtime/render/IWindow.h>
#include <cassert>
#include <chrono>
#include <set>
#include <utility>

namespace ars::engine {
std::string engine::IApplication::get_name() const {
    return "";
}

void IApplication::init(render::IWindow *window) {
    if (_window != nullptr) {
        ARS_LOG_ERROR("Should not init the application twice.");
        return;
    }
    _window = window;
    _window->set_imgui_callback([this]() { on_imgui(); });
}

render::IWindow *IApplication::window() const {
    return _window;
}

void IApplication::quit() {
    _want_to_quit = true;
}

bool IApplication::want_to_quit() const {
    return _want_to_quit;
}

namespace {
class Engine;

Engine *s_engine = nullptr;

class SecondaryWindow {
  public:
    SecondaryWindow(std::unique_ptr<render::IWindow> window,
                    WindowRenderCallback render)
        : _window(std::move(window)), _render(std::move(render)) {}

    void update(double dt) {
        _render(_window.get(), dt);
    }

    [[nodiscard]] bool should_close() const {
        return _window->should_close();
    }

  private:
    std::unique_ptr<render::IWindow> _window{};
    WindowRenderCallback _render{};
};

class Engine {
  public:
    explicit Engine(std::unique_ptr<IApplication> app)
        : _application(std::move(app)) {}

    void create_window(const char *title,
                       uint32_t logical_width,
                       uint32_t logical_height,
                       WindowRenderCallback render) {
        render::WindowInfo info{};
        info.title = title;
        info.logical_size.width = logical_width;
        info.logical_size.height = logical_height;
        auto window = _render_context->create_window(info);
        _secondary_windows.insert(std::make_unique<SecondaryWindow>(
            std::move(window), std::move(render)));
    }

    void run() {
        init_render();

        _application->init(_main_window.get());
        _application->start();

        auto start_time = std::chrono::high_resolution_clock::now();
        auto current_time = start_time;

        while (!_application->want_to_quit() && !_main_window->should_close()) {
            check_secondary_windows_should_close();

            if (_render_context->begin_frame()) {
                auto last_time = current_time;
                current_time = std::chrono::high_resolution_clock::now();
                auto delta_time =
                    std::chrono::duration<double>(current_time - last_time)
                        .count();

                _application->update(delta_time);

                for (auto &w : _secondary_windows) {
                    w->update(delta_time);
                }

                _render_context->end_frame();
            }
        }

        // close all secondary windows if the main window should close.
        // secondary windows should be closed before the application is
        // destroyed, as these windows may have references to resources of the
        // application.
        _secondary_windows.clear();

        _application->destroy();
        _application.reset();

        destroy_render();
    }

    [[nodiscard]] render::IContext *render_context() const {
        return _render_context.get();
    }

  private:
    void check_secondary_windows_should_close() {
        for (auto it = _secondary_windows.begin();
             it != _secondary_windows.end();) {
            if (it->get()->should_close()) {
                it = _secondary_windows.erase(it);
            } else {
                ++it;
            }
        }
    }

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
        _main_window = std::move(window);
    }

    void destroy_render() {
        _main_window.reset();
        _render_context.reset();

        render::destroy_render_backend();
    }

    std::unique_ptr<IApplication> _application{};
    std::unique_ptr<render::IContext> _render_context{};
    std::unique_ptr<render::IWindow> _main_window{};
    std::set<std::unique_ptr<SecondaryWindow>> _secondary_windows{};
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

void create_window(const char *title,
                   uint32_t logical_width,
                   uint32_t logical_height,
                   WindowRenderCallback render) {
    assert(s_engine != nullptr);
    s_engine->create_window(
        title, logical_width, logical_height, std::move(render));
}

render::IContext *render_context() {
    assert(s_engine != nullptr);
    return s_engine->render_context();
}

} // namespace ars::engine