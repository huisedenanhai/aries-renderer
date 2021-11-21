#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Res.h>
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

struct Hello {
    Hello() {
        ARS_LOG_INFO("Init");
    }
    ~Hello() {
        ARS_LOG_INFO("Dispose");
    }
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
        auto t = rttr::type::get<MyComponent>();
        for (auto &prop : t.get_properties()) {
            ARS_LOG_INFO("name: {}", prop.get_name().to_string());
        }

        _shared_ptr_var = std::make_shared<Hello>();

        ARS_LOG_INFO("{}", canonical_res_path(""));
        ARS_LOG_INFO("{}", canonical_res_path("////HE/sc"));
        ARS_LOG_INFO("{}", canonical_res_path("/Era///s//s////"));
        ARS_LOG_INFO("{}", canonical_res_path("/Era///s//s"));
        auto s = split_by(":A:er::re:", ":");
        ARS_LOG_INFO("{}", join(s.begin(), s.end(), ", "));
    }

    void update(double dt) override {
        _scene->update_cached_world_xform();
        window()->present(nullptr);
    }

    void on_imgui() override {
        ImGui::Begin("Hierarchy");
        engine::editor::hierarchy_inspector(_scene.get(), _current_selected);
        ImGui::End();

        ImGui::Begin("Entity");
        engine::editor::entity_inspector(_current_selected);
        ImGui::End();
    }

  private:
    rttr::variant _shared_ptr_var{};
    Resources _resources{};
    std::unique_ptr<engine::Scene> _scene{};
    engine::Entity *_current_selected = nullptr;
};

int main() {
    engine::start_engine(std::make_unique<HierarchyInspectorApplication>());
}