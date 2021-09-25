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
        auto tex = load_texture(ctx, "test.jpg");

        int window_num = 3;
        for (int i = 0; i < window_num; i++) {
            auto title = "Playground Render " + std::to_string(i);
            ars::engine::create_window(
                title.c_str(),
                tex->width() / 2,
                tex->height() / 2,
                [tex](IWindow *window) { window->present(tex.get()); });
        }

        auto model = load_gltf(ctx, "FlightHelmet/FlightHelmet.gltf");
    }

    void update(IWindow *window) override {
        window->present(nullptr);
    }

    void on_imgui() override {
        ImGui::ShowDemoWindow();
    }

    void destroy() override {}

  private:
    std::vector<std::shared_ptr<IRenderObject>> objects{};
};

int main() {
    ars::engine::start_engine(std::make_unique<Application>());
    return 0;
}