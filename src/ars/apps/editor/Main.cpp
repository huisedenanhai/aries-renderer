#include "Entity.h"
#include <ars/runtime/core/Log.h>
#include <ars/runtime/engine/Engine.h>
#include <ars/runtime/engine/Entity.Editor.h>
#include <ars/runtime/engine/Entity.h>
#include <ars/runtime/engine/components/RenderSystem.h>
#include <ars/runtime/render/res/Model.h>
#include <ars/runtime/render/res/Texture.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include "FileBrowser.h"
#include "SaveAsModal.h"
#include "Scene3DView.h"

using namespace ars;

constexpr const char *ARS_3D_VIEW_ID = "3D View";
constexpr const char *ARS_HIERARCHY_INSPECTOR_ID = "Hierarchy";
constexpr const char *ARS_ENTITY_INSPECTOR_ID = "Entity Inspector";
constexpr const char *ARS_FILE_BROWSER_ID = "File Browser";

constexpr const char *ARS_SPAWNABLE_EXTENSION = ".aspawn";

class Editor : public engine::IApplication {
  public:
    engine::IApplication::Info get_info() const override {
        Info info{};
        info.name = "Aries Engine";
        return info;
    }

    void start() override {
        _have_stored_layout = std::filesystem::exists("imgui.ini");
        auto &io = ImGui::GetIO();
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        auto res = ars::engine::resources();
        res->mount("/", std::make_shared<ars::FolderDataProvider>(".ars"));

        _light_bulb_icon =
            render::load_texture(engine::render_context(), "light-bulb.png");

        edit_scene(std::nullopt);

        _file_browser_state.on_file_open =
            [&](const std::filesystem::path &path) {
                auto ext = path.extension();
                if (ext == ".gltf" || ext == ARS_SPAWNABLE_EXTENSION) {
                    edit_scene(path);
                }
            };
    }

    auto reset_edit_scene() {
        _view.reset();
        _scene.reset();
        _scene_save_dir = std::nullopt;
        _current_selected_entity = nullptr;
    }

    void edit_scene(const std::optional<std::filesystem::path> &path) {
        // on_imgui requires current view to be alive. This method will destroy
        // current view, delay it to next frame update.
        _update_tasks.emplace_back([this, path]() {
            reset_edit_scene();

            auto ctx = engine::render_context();
            _scene = std::make_unique<engine::Scene>();
            _view = _scene->render_system()->render_scene()->create_view(
                window()->physical_size());
            _view->set_xform(
                math::XformTRS<float>::from_translation({0, 0.3f, 2.0f}));
            _3d_view_state.focus_distance = 2.0f;

            _view->overlay()->set_light_gizmo(_light_bulb_icon, 0.1f);

            if (path.has_value()) {
                auto ext = path->extension();
                if (ext == ".gltf") {
                    auto model = render::load_gltf(ctx, path.value());
                    engine::load_model(_scene->root(), model);
                }
                if (ext == ARS_SPAWNABLE_EXTENSION) {
                    ars::editor::load_entity(_scene->root(), path.value());
                    _scene_save_dir = path;
                }
            }
        });
    }

    void flush_update_tasks() {
        for (auto &t : _update_tasks) {
            t();
        }
        _update_tasks.clear();
    }

    void update(double dt) override {
        flush_update_tasks();
        _scene->update();
        window()->present(nullptr);
    }

    void save_current_scene() {
        if (_scene_save_dir.has_value()) {
            ars::editor::save_entity(_scene->root(), *_scene_save_dir);
        } else {
            editor::open_save_as_modal(_save_as_state,
                                       "Save As",
                                       ARS_SPAWNABLE_EXTENSION,
                                       _scene->root());
        }
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
                              window(),
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

        ImGui::Begin(ARS_FILE_BROWSER_ID);
        editor::file_browser(_file_browser_state, ".", _current_selected_file);
        ImGui::End();

        bool need_reset_layout = _is_first_imgui_frame && !_have_stored_layout;
        if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Save")) {
                    save_current_scene();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Options")) {
                if (ImGui::MenuItem("Reset Default Layout")) {
                    need_reset_layout = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        editor::save_as_modal(_save_as_state, _scene_save_dir);

        if (need_reset_layout) {
            build_default_dock_layout(dock_space_id);
        }
        ImGui::End();

        _is_first_imgui_frame = false;
    }

    void destroy() override {
        reset_edit_scene();
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
        auto dock_file_browser = ImGui::DockBuilderSplitNode(
            dock_id_3d_view, ImGuiDir_Down, 0.30f, nullptr, &dock_id_3d_view);

        ImGui::DockBuilderDockWindow(ARS_3D_VIEW_ID, dock_id_3d_view);
        ImGui::DockBuilderDockWindow(ARS_HIERARCHY_INSPECTOR_ID, dock_main_id);
        ImGui::DockBuilderDockWindow(ARS_ENTITY_INSPECTOR_ID,
                                     dock_id_inspector);
        ImGui::DockBuilderDockWindow(ARS_FILE_BROWSER_ID, dock_file_browser);
        ImGui::DockBuilderFinish(dock_space_id);
    }

    bool _have_stored_layout = false;
    bool _is_first_imgui_frame = true;

    std::vector<std::function<void()>> _update_tasks{};

    std::shared_ptr<render::ITexture> _light_bulb_icon{};
    std::unique_ptr<render::IView> _view{};
    std::unique_ptr<engine::Scene> _scene{};
    std::optional<std::filesystem::path> _scene_save_dir{};

    editor::Scene3DViewState _3d_view_state{};
    editor::FileBrowserState _file_browser_state{};
    editor::SaveAsModalState _save_as_state{};
    std::filesystem::path _current_selected_file{};
    engine::Entity *_current_selected_entity = nullptr;
};

int main() {
    engine::start_engine(std::make_unique<Editor>());
}