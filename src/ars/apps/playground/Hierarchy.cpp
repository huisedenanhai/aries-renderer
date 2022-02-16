#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Reflect.h>
#include <ars/runtime/core/ResData.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/engine/Entity.Editor.h>
#include <ars/runtime/engine/gui/ImGui.h>
#include <ars/runtime/render/IScene.h>
#include <ars/runtime/render/IWindow.h>
#include <imgui/imgui.h>
#include <utility>

using namespace ars;

struct MyStruct {
    std::string name = "Alice";
    int age = 0;
};

class MyComponent : public engine::IComponent {
    RTTR_DERIVE(engine::IComponent);

  public:
    static void register_component() {

        engine::register_component<MyComponent>("MyComponent")
            .property("data", &MyComponent::data)
            .property("data_hidden", &MyComponent::data_hidden)( //
                rttr::metadata(PropertyAttribute::Display,
                               PropertyDisplay::None))
            .property("my_int", &MyComponent::my_int)
            .property("my_uint32", &MyComponent::my_uint32)
            .property("my_float", &MyComponent::my_float)
            .property("my_vec2", &MyComponent::my_vec2)
            .property("my_vec3", &MyComponent::my_vec3)
            .property("my_vec4", &MyComponent::my_vec4)
            .property("my_rotation", &MyComponent::my_rotation)
            .property("my_xform", &MyComponent::my_xform)
            .property("my_strings", &MyComponent::my_strings)
            .property("my_colors", &MyComponent::my_colors)( //
                rttr::metadata(PropertyAttribute::Display,
                               PropertyDisplay::Color))
            .property("my_struct", &MyComponent::my_struct)
            .property("my_struct_vec", &MyComponent::my_struct_vec);

        rttr::registration::class_<MyStruct>("MyStruct")
            .property("name", &MyStruct::name)
            .property("age", &MyStruct::age);
    }

    ~MyComponent() override {
        ARS_LOG_INFO("Delete MyComponent");
    }

    void on_inspector() override {
        if (ImGui::Button("Dump")) {
            std::stringstream ss;
            ss << std::setw(2) << serialize();
            ARS_LOG_INFO("MyComponent: {}", ss.str());
        }
        ImGui::SameLine();
        if (ImGui::Button("Reload")) {
            auto js = serialize();
            *this = {};
            deserialize(js);
        }
        IComponent::on_inspector();
    }

    void init(engine::Entity *entity) override {
        ARS_LOG_INFO("Init MyComponent");
    }

    void destroy() override {
        ARS_LOG_INFO("Destroy MyComponent");
    }

    std::string data{};
    std::string data_hidden{};
    int my_int = 12;
    uint32_t my_uint32 = 77;
    float my_float = 24.0f;
    glm::vec2 my_vec2{};
    glm::vec3 my_vec3{};
    glm::vec4 my_vec4{};
    glm::quat my_rotation{};
    math::XformTRS<float> my_xform{};
    std::vector<std::string> my_strings{"a", "b", "c"};
    std::array<glm::vec3, 4> my_colors{};
    MyStruct my_struct{};
    std::vector<MyStruct> my_struct_vec{};
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

        ResData res{};
        res.ty = "Hello";
        res.meta = {
            {"a", "Hello"},
            {"b", "World"},
        };
        res.data = {0, 1, 2, 3, 4, 5, 6};

        res.save("test.ares");

        ResData res2{};
        res2.load("test.ares");
        std::stringstream ss;
        for (auto &d : res2.data) {
            ss << static_cast<int>(d) << ", ";
        }
        ARS_LOG_INFO("res2 ty = {}, meta = {}, data = [{}]",
                     res2.ty,
                     res2.meta.dump(),
                     ss.str());
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