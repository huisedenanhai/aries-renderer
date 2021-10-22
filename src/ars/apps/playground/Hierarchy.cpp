#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/scene/Entity.Editor.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/render/IWindow.h>
#include <imgui/imgui.h>

class HierarchyInspectorApplication : public ars::engine::IApplication {
  public:
    [[nodiscard]] std::string get_name() const override {
        return "Hierarchy Inspector";
    }

    void start() override {
        _scene = std::make_unique<ars::scene::Scene>();
        _hierarchy_inspector =
            std::make_unique<ars::scene::editor::HierarchyInspector>();
        _hierarchy_inspector->set_scene(_scene.get());
        _entity_inspector =
            std::make_unique<ars::scene::editor::EntityInspector>();
    }

    void update(double dt) override {
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
    std::unique_ptr<ars::scene::Scene> _scene{};
    std::unique_ptr<ars::scene::editor::HierarchyInspector>
        _hierarchy_inspector{};
    std::unique_ptr<ars::scene::editor::EntityInspector> _entity_inspector{};
};

int main() {
    ars::engine::start_engine(
        std::make_unique<HierarchyInspectorApplication>());
}