#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/engine/Entity.Editor.h>
#include <ars/runtime/engine/Entity.h>
#include <ars/runtime/engine/components/RenderSystem.h>
#include <ars/runtime/render/IWindow.h>
#include <ars/runtime/render/res/Model.h>
#include <ars/runtime/render/res/Texture.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "Scene3DView.h"

using namespace ars;

constexpr const char *ARS_3D_VIEW_ID = "3D View";
constexpr const char *ARS_HIERARCHY_INSPECTOR_ID = "Hierarchy";
constexpr const char *ARS_ENTITY_INSPECTOR_ID = "Entity Inspector";

class Editor : public engine::IApplication {
  public:
    engine::IApplication::Info get_info() const override {
        Info info{};
        info.name = "Aries Engine";
        return info;
    }

    void start() override {
        auto ctx = engine::render_context();
        _have_stored_layout = std::filesystem::exists("imgui.ini");

        auto model = render::load_gltf(
            ctx, "FlightHelmetWithLight/FlightHelmetWithLight.gltf");
        _scene = std::make_unique<engine::Scene>();
        _view = _scene->render_system()->render_scene()->create_view(
            window()->physical_size());
        _view->set_xform(
            math::XformTRS<float>::from_translation({0, 0.3f, 2.0f}));

        engine::load_model(_scene->root(), model);
        _hierarchy_inspector =
            std::make_unique<engine::editor::HierarchyInspector>();
        _entity_inspector = std::make_unique<engine::editor::EntityInspector>();
        _scene_3d_view = std::make_unique<editor::Scene3DView>();
    }

    void update(double dt) override {
        _scene->update();
        window()->present(nullptr);
    }

    void on_imgui() override {
        ImGuiWindowFlags window_flags =
            ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        const ImGuiViewport *viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);
        ImGui::SetNextWindowViewport(viewport->ID);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        window_flags |= ImGuiWindowFlags_NoTitleBar |
                        ImGuiWindowFlags_NoCollapse |
                        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                        ImGuiWindowFlags_NoBringToFrontOnFocus |
                        ImGuiWindowFlags_NoNavFocus;

        ImGui::Begin("Aries Editor", nullptr, window_flags);
        ImGui::PopStyleVar(3);

        auto dock_space_id = ImGui::GetID("Dock Space");
        ImGui::DockSpace(dock_space_id);

        float framebuffer_scale =
            static_cast<float>(window()->physical_size().width) /
            static_cast<float>(window()->logical_size().width);
        ImGui::Begin(ARS_3D_VIEW_ID);
        _scene_3d_view->set_scene(_scene.get());
        _scene_3d_view->set_selected(_hierarchy_inspector->current_selected());
        _scene_3d_view->set_view(_view.get());
        _scene_3d_view->set_framebuffer_scale(framebuffer_scale);
        _scene_3d_view->on_imgui();
        ImGui::End();

        ImGui::Begin(ARS_ENTITY_INSPECTOR_ID);
        _entity_inspector->set_entity(_hierarchy_inspector->current_selected());
        _entity_inspector->on_imgui();
        ImGui::End();

        ImGui::Begin(ARS_HIERARCHY_INSPECTOR_ID);
        _hierarchy_inspector->set_scene(_scene.get());
        _hierarchy_inspector->on_imgui();
        ImGui::End();

        bool need_reset_layout = _is_first_imgui_frame && !_have_stored_layout;
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("Options")) {
                if (ImGui::MenuItem("Reset Default Layout")) {
                    need_reset_layout = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        if (need_reset_layout) {
            build_default_dock_layout(dock_space_id);
        }
        ImGui::End();

        _is_first_imgui_frame = false;
    }

    void destroy() override {
        _view.reset();
        _scene.reset();
    }

  private:
    static void build_default_dock_layout(ImGuiID dock_space_id) {
        // Clear out existing layout
        ImGui::DockBuilderRemoveNode(dock_space_id);
        ImGui::DockBuilderAddNode(
            dock_space_id, ImGuiDockNodeFlags_DockSpace); // Add empty node
        ImGui::DockBuilderSetNodeSize(dock_space_id, ImGui::GetWindowSize());

        auto dock_main_id = dock_space_id;
        auto dock_id_3d_view = ImGui::DockBuilderSplitNode(
            dock_main_id, ImGuiDir_Left, 0.80f, nullptr, &dock_main_id);
        auto dock_id_inspector = ImGui::DockBuilderSplitNode(
            dock_main_id, ImGuiDir_Down, 0.50f, nullptr, &dock_main_id);

        ImGui::DockBuilderDockWindow(ARS_3D_VIEW_ID, dock_id_3d_view);
        ImGui::DockBuilderDockWindow(ARS_HIERARCHY_INSPECTOR_ID, dock_main_id);
        ImGui::DockBuilderDockWindow(ARS_ENTITY_INSPECTOR_ID,
                                     dock_id_inspector);
        ImGui::DockBuilderFinish(dock_space_id);
    }

    bool _have_stored_layout = false;
    bool _is_first_imgui_frame = true;

    std::unique_ptr<render::IView> _view{};
    std::unique_ptr<engine::Scene> _scene{};
    std::unique_ptr<engine::editor::HierarchyInspector> _hierarchy_inspector{};
    std::unique_ptr<engine::editor::EntityInspector> _entity_inspector{};
    std::unique_ptr<editor::Scene3DView> _scene_3d_view{};
};

int main() {
    engine::start_engine(std::make_unique<Editor>());
}