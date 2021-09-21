#include <ars/runtime/core/Log.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ITexture.h>
#include <ars/runtime/render/IWindow.h>
#include <chrono>
#include <imgui/imgui.h>
#include <iostream>
#include <set>
#include <sstream>
#include <stb_image.h>
#include <string>

using namespace ars::render;

std::unique_ptr<ITexture> load_texture(IContext *context,
                                       const std::string &path) {

    using namespace std::chrono;
    auto start = high_resolution_clock::now();
    int width, height, channels;

    unsigned char *data =
        stbi_load(path.c_str(), &width, &height, &channels, 4);

    if (!data) {
        std::stringstream ss;
        ss << "Failed to load image " << path;
        ars::panic(ss.str());
    }

    auto texture =
        context->create_texture_2d(Format::R8G8B8A8_SRGB, width, height);

    auto mid = high_resolution_clock::now();

    texture->set_data(
        data, width * height * 4, 0, 0, 0, 0, 0, width, height, 1);
    texture->generate_mipmap();

    stbi_image_free(data);

    auto stop = high_resolution_clock::now();
    auto total_duration = duration_cast<milliseconds>(stop - start);
    auto decode_duration = duration_cast<milliseconds>(mid - start);
    auto upload_duration = duration_cast<milliseconds>(stop - mid);

    std::cout << "Load texture " << path << " " << width << "x" << height
              << " takes " << total_duration.count()
              << "ms, decode image takes " << decode_duration.count()
              << "ms, upload takes " << upload_duration.count() << "ms"
              << std::endl;

    return texture;
}

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

        int window_num = 4;
        for (int i = 0; i < window_num; i++) {
            auto title = "Playground Render " + std::to_string(i);
            WindowInfo info{};
            info.title = title;
            auto window = ctx->create_window(info);
            _windows.insert(std::move(window));
        }

        _texture = load_texture(ctx, "test.jpg");
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
    std::unique_ptr<ITexture> _texture{};
    std::set<std::unique_ptr<IWindow>> _windows{};
};

int main() {
    ars::engine::start_engine(std::make_unique<Application>());
    return 0;
}