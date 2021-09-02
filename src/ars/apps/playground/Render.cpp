#include <GLFW/glfw3.h>

#include <ars/runtime/render/IRenderContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ISwapchain.h>
#include <iostream>

using namespace ars::render;

int main() {
    std::cout << "Hello World" << std::endl;
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window =
        glfwCreateWindow(800, 600, "Playground Render", nullptr, nullptr);

    ApplicationInfo app_info{};
    app_info.app_name = "Playground Render";
//    app_info.enable_validation = true;

    init_render_backend(app_info);

    auto rd_context = IRenderContext::create(window);
    auto scene = rd_context->create_scene();
    auto view = scene->create_view();

    auto swapchain = rd_context->create_swapchain(window);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        view->render();
        swapchain->present(view->get_color_texture());
    }

    glfwTerminate();
    return 0;
}