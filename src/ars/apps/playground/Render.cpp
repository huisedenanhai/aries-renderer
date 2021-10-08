#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/render/IContext.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/ITexture.h>
#include <ars/runtime/render/IWindow.h>
#include <ars/runtime/render/res/Model.h>
#include <ars/runtime/render/res/Texture.h>
#include <chrono>
#include <imgui/imgui.h>
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

        int window_num = 0;
        for (int i = 0; i < window_num; i++) {
            auto title = "Playground Render " + std::to_string(i);
            ars::engine::create_window(
                title.c_str(),
                tex->width() / 2,
                tex->height() / 2,
                [tex](IWindow *window) { window->present(tex.get()); });
        }

        auto model = load_gltf(ctx, "FlightHelmet/FlightHelmet.gltf");
        _scene = ctx->create_scene();
        _view = _scene->create_view(window()->physical_size());
        _camera_transform.set_translation({0, 0.3f, 2.0f});

        load_render_objects(model);
    }

    void load_render_objects(const Model &model) {
        if (model.scenes.empty()) {
            return;
        }
        auto &scene = model.scenes[model.default_scene.value_or(0)];
        for (auto n : scene.nodes) {
            load_render_objects(model, n, {});
        }
    }

    void
    load_render_objects(const Model &model,
                        Model::Index node,
                        const ars::math::XformTRS<float> &parent_to_world) {
        auto &n = model.nodes[node];
        auto xform = parent_to_world * n.local_to_parent;
        if (n.mesh.has_value()) {
            auto &m = model.meshes[n.mesh.value()];
            for (auto &p : m.primitives) {
                auto rd_obj = _scene->create_render_object();
                rd_obj->set_xform(xform);
                rd_obj->set_mesh(p.mesh);

                if (p.material.has_value()) {
                    auto &mat = model.materials[p.material.value()];
                    rd_obj->set_material(mat.material);
                }

                _objects.emplace_back(std::move(rd_obj));
            }
        }

        for (auto child : n.children) {
            load_render_objects(model, child, xform);
        }
    }

    void update() override {
        _view->set_size(window()->physical_size());
        _view->set_xform(_camera_transform);
        _view->render();
        window()->present(_view->get_color_texture());
    }

    void on_imgui() override {
        ImGui::ShowDemoWindow();
    }

    void destroy() override {
        _objects.clear();
        _view.reset();
        _scene.reset();
    }

  private:
    ars::math::XformTRS<float> _camera_transform{};
    std::vector<std::unique_ptr<IRenderObject>> _objects{};
    std::unique_ptr<IView> _view{};
    std::unique_ptr<IScene> _scene{};
};

int main() {
    ars::engine::start_engine(std::make_unique<Application>());
    return 0;
}