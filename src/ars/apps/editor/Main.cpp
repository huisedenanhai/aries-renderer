#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/engine/Entity.Editor.h>
#include <ars/runtime/engine/Entity.h>
#include <ars/runtime/engine/components/RenderSystem.h>
#include <ars/runtime/render/IWindow.h>
#include <ars/runtime/render/res/Model.h>
#include <ars/runtime/render/res/Texture.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <imguizmo/ImGuizmo.h>

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

        auto light_bulb_icon = render::load_texture(ctx, "light-bulb.png");
        _view->overlay()->set_light_gizmo(light_bulb_icon, 0.1f);

        engine::load_model(_scene->root(), model);

        auto &io = ImGui::GetIO();
        io.ConfigWindowsMoveFromTitleBarOnly = true;
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
        ImGui::Begin(ARS_3D_VIEW_ID, nullptr, ImGuiWindowFlags_MenuBar);
        editor::scene_3d_view(_3d_view_state,
                              _scene.get(),
                              _view.get(),
                              framebuffer_scale,
                              _current_selected_entity);
        ImGui::End();

        ImGui::Begin(ARS_ENTITY_INSPECTOR_ID);
        engine::editor::entity_inspector(_current_selected_entity);
        ImGui::End();

        ImGui::Begin(ARS_HIERARCHY_INSPECTOR_ID);
        engine::editor::hierarchy_inspector(_scene.get(),
                                            _current_selected_entity);
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
    editor::Scene3DViewState _3d_view_state{};
    engine::Entity *_current_selected_entity = nullptr;
};

int main() {
    engine::start_engine(std::make_unique<Editor>());
}