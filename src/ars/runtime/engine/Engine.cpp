#include "Engine.h"
#include "components/RenderSystem.h"
#include <ars/runtime/core/Core.h>
#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Profiler.h>
#include <ars/runtime/core/ResData.h>
#include <ars/runtime/core/Serde.h>
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IWindow.h>
#include <ars/runtime/render/res/Material.h>
#include <ars/runtime/render/res/Mesh.h>
#include <ars/runtime/render/res/Texture.h>
#include <cassert>
#include <chrono>
#include <set>

namespace ars::engine {
IApplication::Info engine::IApplication::get_info() const {
    Info info{};
    return info;
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
        init_core();
        init_render();
        init_resources();
        RenderSystem::register_components();

        _application->init(_main_window.get());
        _application->start();

        auto start_time = std::chrono::high_resolution_clock::now();
        auto current_time = start_time;

        while (!_application->want_to_quit() && !_main_window->should_close()) {
            ARS_PROFILER_NEW_FRAME();
            ARS_PROFILER_SAMPLE("Main Loop", 0xFFAA1639);
            check_secondary_windows_should_close();

            if (_render_context->begin_frame()) {
                auto last_time = current_time;
                current_time = std::chrono::high_resolution_clock::now();
                auto delta_time =
                    std::chrono::duration<double>(current_time - last_time)
                        .count();

                {
                    ARS_PROFILER_SAMPLE("Application Update", 0xFF533421);
                    _application->update(delta_time);
                }

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
        destroy_core();
    }

    [[nodiscard]] render::IContext *render_context() const {
        return _render_context.get();
    }

    [[nodiscard]] Resources *resources() const {
        return _resources.get();
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

        auto info = _application->get_info();
        render::ApplicationInfo app_info{};
        app_info.app_name = info.name;
        app_info.enable_validation = true;
        app_info.enable_profiler = profiler_inited();

        init_render_backend(app_info);

        WindowInfo win_info{};
        win_info.title = info.name;
        win_info.logical_size = {
            info.default_window_logical_width,
            info.default_window_logical_height,
        };
        auto [ctx, window] = IContext::create(&win_info);

        _render_context = std::move(ctx);
        _main_window = std::move(window);
    }

    void init_resources() {
        _resources = std::make_unique<Resources>();
        auto ctx = _render_context.get();
        auto res = _resources.get();
        res->register_loader<render::ITexture>(
            ars::make_loader([ctx](const ars::ResData &data) {
                return load_texture(ctx, data);
            }));
        res->register_loader<render::IMesh>(ars::make_loader(
            [ctx](const ars::ResData &data) { return load_mesh(ctx, data); }));
        res->register_loader<render::IMaterial>(
            ars::make_loader([ctx, res](const ars::ResData &data) {
                return load_material(ctx, res, data);
            }));

        ars::set_serde_res_provider(_resources.get());
    }

    void destroy_render() {
        _main_window.reset();
        _resources.reset();
        _render_context.reset();

        render::destroy_render_backend();
    }

    std::unique_ptr<IApplication> _application{};
    std::unique_ptr<Resources> _resources{};
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

Resources *resources() {
    assert(s_engine != nullptr);
    return s_engine->resources();
}
} // namespace ars::engine