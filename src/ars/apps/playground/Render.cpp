#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Profiler.h>
#include <ars/runtime/core/input/Keyboard.h>
#include <ars/runtime/core/input/Mouse.h>
#include <ars/runtime/core/misc/Defer.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/engine/Entity.Editor.h>
#include <ars/runtime/engine/Entity.h>
#include <ars/runtime/engine/components/RenderSystem.h>
#include <ars/runtime/engine/gui/ImGui.h>
#include <ars/runtime/render/IEffect.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ITexture.h>
#include <ars/runtime/render/IWindow.h>
#include <ars/runtime/render/res/Material.h>
#include <ars/runtime/render/res/Mesh.h>
#include <ars/runtime/render/res/Model.h>
#include <ars/runtime/render/res/Texture.h>
#include <imgui/imgui.h>
#include <stb_image.h>
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

        auto res = ars::engine::resources();
        res->mount("/", std::make_shared<ars::FolderDataProvider>(".ars"));

        tex = res->load<ITexture>("test.jpg");

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

        // auto model_file = "FlightHelmetWithLight/FlightHelmetWithLight.gltf";
        // auto model_file = "Balls/Balls.gltf";
        // auto model_file = "Hexagon/Hexagon.gltf";
        auto model_file = "SSRTest/SSR.gltf";
        auto model = load_gltf(ctx, model_file);
        _scene = std::make_unique<ars::engine::Scene>();
        _view = _scene->render_system()->render_scene()->create_view(
            window()->physical_size());
        auto light_bulb_icon = ars::render::load_texture(ctx, "light-bulb.png");
        _view->overlay()->set_light_gizmo(light_bulb_icon, 0.1f);
        ars::engine::load_model(_scene->root(), model);

        _fly_camera.xform.set_translation({0, 0.3f, 10.0f});
        for (auto e : _scene->entities()) {
            auto camera = e->component<ars::engine::Camera>();
            if (camera != nullptr) {
                auto xform = e->world_xform();
                _fly_camera.xform = xform;
                _view->set_camera(camera->data());
                break;
            }
        }

        for (auto &node : model.nodes) {
            if (node.camera.has_value()) {
                _view->set_camera(model.cameras[node.camera.value()].data);
            }
        }

        // load_test_mesh();
        // test_load_cube_map();

        std::shared_ptr<ITexture> hdr_tex{};
        {
            auto hdr_file = "Environments/studio_garden_2k.hdr";
            // auto hdr_file = "Environments/skybox_room.hdr";
            // auto hdr_file = "Environments/studio_small_08_2k.hdr";
            // auto hdr_file = "Environments/dreifaltigkeitsberg_2k.hdr";
            int w, h, c;
            auto data = stbi_loadf(hdr_file, &w, &h, &c, 4);
            ARS_DEFER([&]() { stbi_image_free(data); });
            auto info =
                TextureInfo::create_2d(Format::R32G32B32A32_SFLOAT, w, h);
            hdr_tex = ctx->create_texture(info);
            hdr_tex->set_data(
                data, w * h * 4 * sizeof(float), 0, 0, 0, 0, 0, w, h, 1);
            hdr_tex->generate_mipmap();
        }

        auto sky = ctx->create_panorama_sky();
        sky->set_panorama(hdr_tex);
        sky->set_irradiance_cube_map_size(256);
        auto background = _view->effect()->background();
        background->set_sky(sky);
        background->set_color({1.0f, 1.0f, 1.0f});
    }

    static void test_load_cube_map() {
        auto ctx = ars::engine::render_context();
        // Test create cube map
        auto cube_map = ctx->create_texture(
            TextureInfo::create_cube_map(Format::R8G8B8A8_SRGB, 256));
        std::vector<std::string> face_names{"px", "nx", "py", "ny", "pz", "nz"};
        for (int i = 0; i < 6; i++) {
            auto face_file_name =
                fmt::format("CubeMaps/SmallStudio/{}.png", face_names[i]);
            int w, h, c;
            auto data = stbi_load(face_file_name.c_str(), &w, &h, &c, 4);
            ARS_DEFER([&]() { stbi_image_free(data); });
            if (w != 256 || h != 256) {
                ARS_LOG_ERROR("This playground require cube map of size 256");
                continue;
            }
            cube_map->set_data(data,
                               w * h * 4 * sizeof(unsigned char),
                               0,
                               i,
                               0,
                               0,
                               0,
                               w,
                               h,
                               1);
        }
        cube_map->generate_mipmap();
    }

    void load_test_mesh() {
        auto res = ars::engine::resources();
        auto e = _scene->create_entity();
        auto mesh_rd = e->add_component<ars::engine::MeshRenderer>();
        auto prim = mesh_rd->add_primitive();
        auto mesh = res->load<IMesh>(
            "FlightHelmetWithLight/FlightHelmetWithLight.gltf/meshes/"
            "RubberWood_low.001.0");
        prim->set_mesh(mesh);

        auto mat = res->load<IMaterial>(
            "FlightHelmetWithLight/FlightHelmetWithLight.gltf/materials/"
            "RubberWoodMat.001");
        prim->set_material(mat);
    }

    void update(double dt) override {
        _scene->update();

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
        if (_current_selected == nullptr) {
            return;
        }
        auto mesh_renderer =
            _current_selected->component<ars::engine::MeshRenderer>();
        if (mesh_renderer == nullptr) {
            return;
        }
        for (int i = 0; i < mesh_renderer->primitive_count(); i++) {
            _view->overlay()->draw_outline(
                0,
                _current_selected->cached_world_xform(),
                mesh_renderer->primitive(i)->mesh());
        }
    }

    void on_imgui() override {
        ImGui::ShowDemoWindow();

        ImGui::Begin("Hierarchy");
        ars::engine::editor::hierarchy_inspector(_scene.get(),
                                                 _current_selected);
        ImGui::End();

        ImGui::Begin("Entity");
        ars::engine::editor::entity_inspector(_current_selected);
        ImGui::End();

        ImGui::Begin("Image");
        ImGui::Image(_view->get_color_texture()->handle(),
                     {
                         _view->size().width * 0.2f,
                         _view->size().height * 0.2f,
                     });
        ImGui::End();

        _view->debug_gui();

        ars::profiler_on_gui("Profiler", _profiler_gui_state);

        ImGui::Begin("Effect");
        ars::gui::input_instance(rttr::instance(_view->effect()));
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
                _current_selected =
                    reinterpret_cast<ars::engine::Entity *>(selection[0]);
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
    ars::engine::Entity *_current_selected = nullptr;
    ars::ProfilerGuiState _profiler_gui_state{};
};

int main() {
    ars::engine::start_engine(std::make_unique<Application>());
    return 0;
}