#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/render/IWindow.h>
#include <ars/runtime/render/res/Texture.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

using namespace ars;

constexpr const char *ARS_3D_VIEW_ID = "3D View";
constexpr const char *ARS_HIERARCHY_VIEW_ID = "Hierarchy";
constexpr const char *ARS_ENTITY_INSPECTOR_ID = "Entity Inspector";

class Editor : public engine::IApplication {
  public:
    [[nodiscard]] std::string get_name() const override {
        return "Aries Editor";
    }

    void start() override {
        auto ctx = engine::render_context();
        _test_tex = render::load_texture(ctx, "test.jpg");
        _have_stored_layout = std::filesystem::exists("imgui.ini");
    }

    void update(double dt) override {
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

        ImGui::Begin(ARS_3D_VIEW_ID);
        ImGui::Image(_test_tex.get(), {256, 128});
        ImGui::End();
        ImGui::Begin(ARS_ENTITY_INSPECTOR_ID);
        ImGui::Image(_test_tex.get(), {256, 128});
        ImGui::End();
        ImGui::Begin(ARS_HIERARCHY_VIEW_ID);
        ImGui::Image(_test_tex.get(), {256, 128});
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

    void build_default_dock_layout(ImGuiID dock_space_id) {
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
        ImGui::DockBuilderDockWindow(ARS_HIERARCHY_VIEW_ID, dock_main_id);
        ImGui::DockBuilderDockWindow(ARS_ENTITY_INSPECTOR_ID,
                                     dock_id_inspector);
        ImGui::DockBuilderFinish(dock_space_id);
    }

  private:
    std::shared_ptr<render::ITexture> _test_tex{};
    bool _have_stored_layout = false;
    bool _is_first_imgui_frame = true;
};

int main() {
    engine::start_engine(std::make_unique<Editor>());
}