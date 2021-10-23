#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/scene/Entity.Editor.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/IWindow.h>
#include <imgui/imgui.h>
#include <rttr/registration>
#include <sstream>
#include <utility>

using namespace ars;

class MeshFeatureProcessor
    : public SoA<scene::Entity *, std::unique_ptr<render::IRenderObject>> {
  public:
};

class MeshRenderer : public scene::IComponent {
  public:
    void init(scene::Entity *entity) override {}

    void on_inspector() override {
        ImGui::Text("Mesh Renderer");
    }

  private:
    MeshFeatureProcessor *_processor{};
    MeshFeatureProcessor::Id _id{};
};

class MyComponent : public scene::IComponent {
    ARS_COMPONENT(MyComponent, scene::IComponent);

  public:
    std::string data{};
};

RTTR_REGISTRATION {
    scene::register_component<MyComponent>("MyComponent")
        .property("data", &MyComponent::data);
}

class HierarchyInspectorApplication : public engine::IApplication {
  public:
    [[nodiscard]] std::string get_name() const override {
        return "Hierarchy Inspector";
    }

    void start() override {
        _scene = std::make_unique<scene::Scene>();
        _hierarchy_inspector =
            std::make_unique<scene::editor::HierarchyInspector>();
        _hierarchy_inspector->set_scene(_scene.get());
        _entity_inspector = std::make_unique<scene::editor::EntityInspector>();

        auto t = rttr::type::get<MyComponent>();
        for (auto &prop : t.get_properties()) {
            std::stringstream ss;
            ss << "name: " << prop.get_name();
            log_info(ss.str());
        }
    }

    void update(double dt) override {
        _scene->update_cached_world_xform();
        window()->present(nullptr);
    }

    void on_imgui() override {
        ImGui::Begin("Hierarchy");
        _hierarchy_inspector->on_imgui();
        ImGui::End();

        ImGui::Begin("Entity");
        _entity_inspector->set_entity(_hierarchy_inspector->current_selected());
        _entity_inspector->on_imgui();
        ImGui::End();
    }

  private:
    std::unique_ptr<scene::Scene> _scene{};
    std::unique_ptr<scene::editor::HierarchyInspector> _hierarchy_inspector{};
    std::unique_ptr<scene::editor::EntityInspector> _entity_inspector{};
};

int main() {
    engine::start_engine(std::make_unique<HierarchyInspectorApplication>());
}