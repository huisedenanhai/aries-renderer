#ifdef __APPLE__
#define GLFW_EXPOSE_NATIVE_COCOA
#define ARS_PLATFORM_MACOS
#elif defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define GLFW_EXPOSE_NATIVE_WIN32
#define ARS_PLATFORM_WINDOWS
#endif
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>

#include <ars/runtime/render/IRenderContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ISwapchain.h>
#include <iostream>

using namespace ars::render;

uint64_t glfwGetNativeHandle(GLFWwindow *window) {
#ifdef ARS_PLATFORM_WINDOWS
    return reinterpret_cast<uint64_t>(glfwGetWin32Window(window));
#elif ARS_PLATFORM_MACOS
    return reinterpret_cast<uint64_t>(glfwGetCocoaWindow(window));
#else
#error "Unsupported platform"
#endif
}

int main() {
    std::cout << "Hello World" << std::endl;
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    auto window =
        glfwCreateWindow(800, 600, "Playground Render", nullptr, nullptr);

    auto handle = glfwGetNativeHandle(window);

    auto rd_context = IRenderContext::create(Backend::Vulkan, handle);
    auto scene = rd_context->create_scene();
    auto view = scene->create_view();

    auto swapchain = rd_context->create_swapchain(handle);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        view->render();
        swapchain->present(view->get_color_texture());
    }

    glfwTerminate();
    return 0;
}