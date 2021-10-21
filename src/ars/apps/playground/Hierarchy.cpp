#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/scene/Entity.Editor.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/render/IWindow.h>
#include <imgui/imgui.h>

class HierarchyInspectorApplication : public ars::engine::IApplication {
  public:
    std::string get_name() const override {
        return "Hierarchy Inspector";
    }

    void start() override {
        _hierarchy_inspector =
            std::make_unique<ars::scene::editor::HierarchyInspector>();
    }

    void update(double dt) override {
        window()->present(nullptr);
    }

    void on_imgui() override {
        ImGui::Begin("Hierarchy");
        _hierarchy_inspector->on_imgui();
        ImGui::End();
    }

  private:
    std::unique_ptr<ars::scene::editor::HierarchyInspector>
        _hierarchy_inspector{};
};

int main() {
    ars::engine::start_engine(
        std::make_unique<HierarchyInspectorApplication>());
}