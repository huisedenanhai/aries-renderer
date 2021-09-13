#include <GLFW/glfw3.h>

#include <ars/runtime/core/Log.h>
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ISwapchain.h>
#include <ars/runtime/render/ITexture.h>
#include <iostream>
#include <set>
#include <sstream>
#include <stb_image.h>
#include <string>

using namespace ars::render;

struct Window {
    GLFWwindow *window{};
    std::unique_ptr<ISwapchain> swapchain{};

    Window(int width,
           int height,
           const std::string &title,
           std::unique_ptr<IContext> &rd_context) {
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        window =
            glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);

        if (rd_context == nullptr) {
            rd_context = IContext::create(window);
        }

        swapchain = rd_context->create_swapchain(window);
    }

    [[nodiscard]] bool should_close() const {
        return glfwWindowShouldClose(window);
    }

    ~Window() {
        swapchain.reset();
        glfwDestroyWindow(window);
    }
};

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

    std::set<std::unique_ptr<Window>> windows{};
    std::unique_ptr<IContext> ctx{};

    int window_num = 4;
    for (int i = 0; i < window_num; i++) {
        windows.insert(std::make_unique<Window>(
            800, 600, "Playground Render " + std::to_string(i), ctx));
    }
    auto scene = ctx->create_scene();
    auto view = scene->create_view();

    auto texture = load_texture(ctx.get(), "test.jpg");

    while (!windows.empty()) {
        glfwPollEvents();

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
#ifdef __APPLE__
                if (!glfwGetWindowAttrib(w->window, GLFW_FOCUSED)) {
                    continue;
                }
#endif
                if (!w->swapchain->present(texture.get())) {
                    int fb_width, fb_height;
                    glfwGetFramebufferSize(w->window, &fb_width, &fb_height);
                    w->swapchain->resize(fb_width, fb_height);
                }
            }

            ctx->end_frame();
        }
    }
}

int main() {
    glfwInit();

    ApplicationInfo app_info{};
    app_info.app_name = "Playground Render";
    app_info.enable_validation = true;

    init_render_backend(app_info);

    main_loop();

    destroy_render_backend();

    glfwTerminate();
    return 0;
}