#include <ars/runtime/core/Log.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/engine/Entity.Editor.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/IWindow.h>
#include <imgui/imgui.h>
#include <rttr/registration>
#include <utility>

using namespace ars;

class MyComponent : public engine::IComponent {
    ARS_COMPONENT(MyComponent, engine::IComponent);

  public:
    static void register_component() {
        engine::register_component<MyComponent>("MyComponent")
            .property("data", &MyComponent::data)
            .property("my_int", &MyComponent::my_int)
            .property("my_float", &MyComponent::my_float)
            .property("my_vec2", &MyComponent::my_vec2)
            .property("my_vec3", &MyComponent::my_vec3)
            .property("my_vec4", &MyComponent::my_vec4)
            .property("my_rotation", &MyComponent::my_rotation)
            .property("my_xform", &MyComponent::my_xform);
    }

    void init(engine::Entity *entity) override {
        ARS_LOG_INFO("Init");
    }

    void destroy() override {
        ARS_LOG_INFO("Destroy");
    }

    std::string data{};
    int my_int = 12;
    float my_float = 24.0f;
    glm::vec2 my_vec2{};
    glm::vec3 my_vec3{};
    glm::vec4 my_vec4{};
    glm::quat my_rotation{};
    math::XformTRS<float> my_xform{};
};

class HierarchyInspectorApplication : public engine::IApplication {
  public:
    engine::IApplication::Info get_info() const override {
        Info info{};
        info.name = "Hierarchy Editor";
        return info;
    }

    void start() override {
        MyComponent::register_component();

        _scene = std::make_unique<engine::Scene>();
        _hierarchy_inspector =
            std::make_unique<engine::editor::HierarchyInspector>();
        _hierarchy_inspector->set_scene(_scene.get());
        _entity_inspector = std::make_unique<engine::editor::EntityInspector>();

        auto t = rttr::type::get<MyComponent>();
        for (auto &prop : t.get_properties()) {
            ARS_LOG_INFO("name: {}", prop.get_name().to_string());
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
    std::unique_ptr<engine::Scene> _scene{};
    std::unique_ptr<engine::editor::HierarchyInspector> _hierarchy_inspector{};
    std::unique_ptr<engine::editor::EntityInspector> _entity_inspector{};
};

int main() {
    engine::start_engine(std::make_unique<HierarchyInspectorApplication>());
}