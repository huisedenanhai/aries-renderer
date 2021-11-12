#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/input/Keyboard.h>
#include <ars/runtime/core/input/Mouse.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/engine/Entity.Editor.h>
#include <ars/runtime/engine/Entity.h>
#include <ars/runtime/engine/components/RenderSystem.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ITexture.h>
#include <ars/runtime/render/IWindow.h>
#include <ars/runtime/render/res/Model.h>
#include <ars/runtime/render/res/Texture.h>
#include <imgui/imgui.h>
#include <string>

using namespace ars::render;

struct FlyCamera {
    ars::math::XformTRS<float> xform{};
    glm::vec3 euler{};

    void handle_input(IWindow *window, double dt) {
        handle_translation(window, dt);
        handle_rotation(window, dt);
    }

  private:
    void handle_translation(IWindow *window, double dt) {
        using namespace ars::input;
        auto keyboard = window->keyboard();
        auto delta_pos = glm::vec3(0.0f);
        float d = 1.0f * static_cast<float>(dt);
        if (keyboard->is_holding(Key::W)) {
            delta_pos += d * xform.forward();
        }
        if (keyboard->is_holding(Key::S)) {
            delta_pos += -d * xform.forward();
        }
        if (keyboard->is_holding(Key::A)) {
            delta_pos += -d * xform.right();
        }
        if (keyboard->is_holding(Key::D)) {
            delta_pos += d * xform.right();
        }
        if (keyboard->is_holding(Key::Q)) {
            delta_pos += -d * xform.up();
        }
        if (keyboard->is_holding(Key::E)) {
            delta_pos += d * xform.up();
        }

        auto pos = xform.translation();
        xform.set_translation(pos + delta_pos);
    }

    void handle_rotation(IWindow *window, double dt) {
        auto mouse = window->mouse();
        float d_rot = 0.5f * static_cast<float>(dt);
        auto cursor_delta = glm::vec2(mouse->cursor_position_delta());
        euler.x -= d_rot * cursor_delta.y;
        euler.y -= d_rot * cursor_delta.x;

        euler.x =
            std::clamp(euler.x, -glm::radians(89.0f), glm::radians(89.0f));
        auto pi = glm::pi<float>();
        while (euler.y >= pi) {
            euler.y -= 2.0f * pi;
        }
        while (euler.y < -pi) {
            euler.y += 2.0f * pi;
        }
        xform.set_rotation(euler);
    }
};

class Application : public ars::engine::IApplication {
  public:
    ars::engine::IApplication::Info get_info() const override {
        Info info{};
        info.name = "Playground Render";
        return info;
    }

    void start() override {
        {
            // test if offscreen context runs
            // auto offscreen_context = IContext::create(nullptr);
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
                [tex](IWindow *window, [[maybe_unused]] double dt) {
                    window->present(tex.get());
                });
        }

        // auto model = load_gltf(ctx, "FlightHelmet/FlightHelmet.gltf");
        // auto model = load_gltf(ctx, "Test/Test.gltf");
        auto model =
            load_gltf(ctx, "FlightHelmetWithLight/FlightHelmetWithLight.gltf");
        _scene = std::make_unique<ars::engine::Scene>();
        _view = _scene->render_system()->render_scene()->create_view(
            window()->physical_size());
        _fly_camera.xform.set_translation({0, 0.3f, 2.0f});
        ars::engine::load_model(_scene->root(), model);

        _hierarchy_inspector =
            std::make_unique<ars::engine::editor::HierarchyInspector>();
        _entity_inspector =
            std::make_unique<ars::engine::editor::EntityInspector>();
    }

    void update(double dt) override {
        _scene->update_cached_world_xform();
        _scene->render_system()->update();

        if (window()->keyboard()->is_released(ars::input::Key::Escape)) {
            quit();
            return;
        }

        auto mouse = window()->mouse();
        if (mouse->is_holding(ars::input::MouseButton::Right)) {
            window()->set_cursor_mode(CursorMode::HiddenCaptured);
            _fly_camera.handle_input(window(), dt);
        } else {
            window()->set_cursor_mode(CursorMode::Normal);
        }

        draw_selected_object_outline();

        _view->set_size(window()->physical_size());
        _view->set_xform(_fly_camera.xform);
        _view->render();
        window()->present(_view->get_color_texture());
    }

    void draw_selected_object_outline() {
        auto selected_entity = _hierarchy_inspector->current_selected();
        if (selected_entity == nullptr) {
            return;
        }
        auto mesh_renderer =
            selected_entity->component<ars::engine::MeshRenderer>();
        if (mesh_renderer == nullptr) {
            return;
        }
        for (int i = 0; i < mesh_renderer->primitive_count(); i++) {
            _view->overlay()->draw_outline(
                0,
                selected_entity->cached_world_xform(),
                mesh_renderer->primitive(i)->mesh());
        }
    }

    void on_imgui() override {
        ImGui::ShowDemoWindow();

        ImGui::Begin("Hierarchy");
        _hierarchy_inspector->set_scene(_scene.get());
        _hierarchy_inspector->on_imgui();
        ImGui::End();

        ImGui::Begin("Entity");
        _entity_inspector->set_entity(_hierarchy_inspector->current_selected());
        _entity_inspector->on_imgui();
        ImGui::End();

        ImGui::Begin("Image");
        ImGui::Image(_view->get_color_texture(),
                     {
                         _view->size().width * 0.2f,
                         _view->size().height * 0.2f,
                     });
        ImGui::End();

        if (ImGui::GetIO().WantCaptureMouse) {
            return;
        }

        auto mouse = window()->mouse();
        if (mouse->is_released(ars::input::MouseButton::Left)) {
            auto pos =
                window()->from_logical_to_physical(mouse->cursor_position());
            auto selection =
                _view->query_selection(static_cast<uint32_t>(pos.x),
                                       static_cast<uint32_t>(pos.y),
                                       1,
                                       1);
            if (!selection.empty()) {
                _hierarchy_inspector->set_current_selected(
                    reinterpret_cast<ars::engine::Entity *>(selection[0]));
            }
        }
    }

    void destroy() override {
        _view.reset();
        _scene.reset();
    }

  private:
    FlyCamera _fly_camera{};
    std::unique_ptr<IView> _view{};
    std::unique_ptr<ars::engine::Scene> _scene{};

    std::unique_ptr<ars::engine::editor::HierarchyInspector>
        _hierarchy_inspector{};
    std::unique_ptr<ars::engine::editor::EntityInspector> _entity_inspector{};
};

int main() {
    ars::engine::start_engine(std::make_unique<Application>());
    return 0;
}