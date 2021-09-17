#include <ars/runtime/core/Log.h>
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ITexture.h>
#include <ars/runtime/render/IWindow.h>
#include <iostream>
#include <set>
#include <sstream>
#include <stb_image.h>
#include <string>

using namespace ars::render;

std::unique_ptr<ITexture> load_texture(IContext *context,
                                       const std::string &path) {
    int width, height, channels;
    unsigned char *data =
        stbi_load(path.c_str(), &width, &height, &channels, 4);
    if (!data) {
        std::stringstream ss;
        ss << "Failed to load image " << path;
        ars::panic(ss.str());
    }

    auto texture =
        context->create_texture_2d(Format::R8G8B8A8Srgb, width, height);
    texture->set_data(
        data, width * height * 4, 0, 0, 0, 0, 0, width, height, 1);
    texture->generate_mipmap();

    stbi_image_free(data);

    return texture;
}

void main_loop() {
    {
        // test if offscreen context runs
        auto offscreen_context = IContext::create(nullptr);
    }

    std::set<std::unique_ptr<IWindow>> windows{};
    std::unique_ptr<IContext> ctx{};

    ars::math::XformTRS<float> trans{};
    auto m = trans.matrix();

    ars::math::XformTRS<float> t(glm::identity<glm::mat4>());

    int window_num = 4;
    for (int i = 0; i < window_num; i++) {
        auto title = "Playground Render " + std::to_string(i);
        WindowInfo info{};
        info.title = title.c_str();

        if (ctx == nullptr) {
            auto [context, window] = IContext::create(&info);
            ctx = std::move(context);
            windows.insert(std::move(window));
        } else {
            windows.insert(ctx->create_window(info));
        }
    }
    auto scene = ctx->create_scene();
    auto view = scene->create_view();

    auto texture = load_texture(ctx.get(), "test.jpg");

    while (!windows.empty()) {

        for (auto it = windows.begin(); it != windows.end();) {
            if (it->get()->should_close()) {
                it = windows.erase(it);
            } else {
                ++it;
            }
        }

        if (ctx->begin_frame()) {
            view->render();

            for (auto &w : windows) {
                w->present(texture.get());
            }

            ctx->end_frame();
        }
    }
}

int main() {
    ApplicationInfo app_info{};
    app_info.app_name = "Playground Render";
    app_info.enable_validation = true;

    init_render_backend(app_info);

    main_loop();

    destroy_render_backend();
    return 0;
}