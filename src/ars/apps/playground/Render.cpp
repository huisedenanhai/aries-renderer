#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ITexture.h>
#include <ars/runtime/render/IWindow.h>
#include <ars/runtime/render/res/Model.h>
#include <ars/runtime/render/res/Texture.h>
#include <chrono>
#include <imgui/imgui.h>
#include <set>
#include <string>

using namespace ars::render;

class Application : public ars::engine::IApplication {
  public:
    [[nodiscard]] std::string get_name() const override {
        return "Playground Render";
    }

    void start() override {
        {
            // test if offscreen context runs
            auto offscreen_context = IContext::create(nullptr);
        }

        auto ctx = ars::engine::render_context();

        ars::math::XformTRS<float> trans{};
        auto m = trans.matrix();

        ars::math::XformTRS<float> t(glm::identity<glm::mat4>());

        int window_num = 3;
        for (int i = 0; i < window_num; i++) {
            auto title = "Playground Render " + std::to_string(i);
            WindowInfo info{};
            info.title = title;
            auto window = ctx->create_window(info);
            _windows.insert(std::move(window));
        }

        _texture = load_texture(ctx, "test.jpg");
        auto model = load_gltf(ctx, "FlightHelmet/FlightHelmet.gltf");
    }

    void update() override {
        for (auto it = _windows.begin(); it != _windows.end();) {
            if (it->get()->should_close()) {
                it = _windows.erase(it);
            } else {
                ++it;
            }
        }

        for (auto &w : _windows) {
            w->present(_texture.get());
        }
    }

    void on_imgui() override {
        ImGui::ShowDemoWindow();
    }

    void destroy() override {}

  private:
    std::shared_ptr<ITexture> _texture{};
    std::set<std::unique_ptr<IWindow>> _windows{};
};

int main() {
    ars::engine::start_engine(std::make_unique<Application>());
    return 0;
}