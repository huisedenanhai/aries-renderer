#include <GLFW/glfw3.h>

#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ISwapchain.h>
#include <iostream>
#include <set>
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

void main_loop() {
    {
        // test if offscreen context runs
        auto offscreen_context = IContext::create(nullptr);
    }

    std::set<std::unique_ptr<Window>> windows{};
    std::unique_ptr<IContext> rd_context{};

    int window_num = 4;
    for (int i = 0; i < window_num; i++) {
        windows.insert(std::make_unique<Window>(
            800, 600, "Playground Render " + std::to_string(i), rd_context));
    }
    auto scene = rd_context->create_scene();
    auto view = scene->create_view();

    while (!windows.empty()) {
        glfwPollEvents();

        for (auto it = windows.begin(); it != windows.end();) {
            if (it->get()->should_close()) {
                it = windows.erase(it);
            } else {
                ++it;
            }
        }

        view->render();

        for (auto &w : windows) {
            w->swapchain->present(view->get_color_texture());
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