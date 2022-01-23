#include "Profiler.h"

#define IMGUI_DEFINE_MATH_OPERATORS
#include <chrono>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

#include <deque>
#include <spdlog/fmt/fmt.h>
#include <stack>
#include <vector>

namespace ars {
namespace {
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

struct Sample {
    std::string name{};
    uint32_t color{};
    float start_time_ms{};
    float end_time_ms{};
    std::vector<std::unique_ptr<Sample>> children{};
    const char *file_name = nullptr;
    uint32_t line = 0;
    const char *function_name = nullptr;
};

struct ProfilerGroup {
    std::string name;
    std::deque<std::unique_ptr<Sample>> samples{};
    std::stack<std::unique_ptr<Sample>> working_stack{};
    bool pause = false;
    uint32_t index = 0;

    void begin_sample(const std::string &sample_name,
                      uint32_t color,
                      float start_time_ms,
                      const char *file_name,
                      uint32_t line,
                      const char *function_name) {
        auto s = std::make_unique<Sample>();
        s->name = sample_name;
        s->color = color;
        s->start_time_ms = start_time_ms;
        s->file_name = file_name;
        s->line = line;
        s->function_name = function_name;

        working_stack.emplace(std::move(s));
    }

    void end_sample(float end_time_ms) {
        assert(!working_stack.empty());
        auto s = std::move(working_stack.top());
        working_stack.pop();
        s->end_time_ms = end_time_ms;

        if (working_stack.empty()) {
            submit_root_sample(std::move(s));
        } else {
            working_stack.top()->children.emplace_back(std::move(s));
        }
    }

    void submit_root_sample(std::unique_ptr<Sample> s) {
        if (pause) {
            return;
        }
        samples.emplace_back(std::move(s));
        if (samples.size() > MAX_PROFILER_SAMPLE_NUM) {
            samples.pop_front();
        }
    }

    static void
    draw_clipped_text(ImDrawList *draw, const ImVec4 rect, const char *text) {
        auto pos = ImVec2(rect.x, rect.y);
        // Draw text twice to fake outline
        draw->AddText(nullptr,
                      0.0,
                      pos + ImVec2(1, 1),
                      0xFF000000,
                      text,
                      nullptr,
                      0.0f,
                      &rect);
        draw->AddText(
            nullptr, 0.0, pos, 0xFFFFFFFF, text, nullptr, 0.0f, &rect);
    }

    void sample_time_ms_min_max(float &sample_min_time_ms,
                                float &sample_max_time_ms) {
        sample_min_time_ms = std::numeric_limits<float>::max();
        sample_max_time_ms = std::numeric_limits<float>::min();

        for (auto &s : samples) {
            sample_min_time_ms = std::min(s->start_time_ms, sample_min_time_ms);
            sample_max_time_ms = std::max(s->end_time_ms, sample_min_time_ms);
        }
    }

    void display_bar_graph(const std::deque<float> &frame_times_ms,
                           float display_min_time_ms,
                           float display_max_time_ms,
                           ProfilerGuiState &state) {
        auto bar_scale = state.bar_scale;
        float bar_height = ImGui::GetTextLineHeightWithSpacing();

        float scroll_rect_width =
            (display_max_time_ms - display_min_time_ms) * bar_scale;

        auto visible_time_range_ms =
            ImGui::GetContentRegionAvailWidth() / bar_scale;
        auto visible_min_time_ms =
            display_min_time_ms +
            state.scroll_x * (display_max_time_ms - display_min_time_ms -
                              visible_time_range_ms);
        auto visible_max_time_ms = visible_min_time_ms + visible_time_range_ms;

        draw_frame_seperator(frame_times_ms,
                             display_min_time_ms,
                             bar_scale,
                             visible_min_time_ms,
                             visible_max_time_ms);

        float height_offset = draw_bars(display_min_time_ms,
                                        bar_scale,
                                        bar_height,
                                        visible_min_time_ms,
                                        visible_max_time_ms);

        ImGui::InvisibleButton(
            "Background",
            ImVec2(scroll_rect_width,
                   std::max(height_offset, ImGui::GetContentRegionAvail().y)));
        if (ImGui::IsItemActive() &&
            ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
            state.scroll_x -= ImGui::GetIO().MouseDelta.x / scroll_rect_width;
            state.scroll_x = std::clamp(state.scroll_x, 0.0f, 1.0f);
        }

        ImGui::SetScrollHereX(state.scroll_x);
    }

    static void draw_frame_seperator(const std::deque<float> &frame_times_ms,
                                     float display_min_time_ms,
                                     float bar_scale,
                                     float visible_min_time_ms,
                                     float visible_max_time_ms) {
        auto draw = ImGui::GetWindowDrawList();
        auto offset = ImGui::GetCursorScreenPos();
        for (auto &frame_ms : frame_times_ms) {
            if (frame_ms < visible_min_time_ms ||
                frame_ms > visible_max_time_ms) {
                continue;
            }
            auto x = (frame_ms - display_min_time_ms) * bar_scale;
            draw->AddLine(offset + ImVec2(x, -1000.0f),
                          offset + ImVec2(x, 1000.0f),
                          0xAAAAAAAA);
        }
    }

    float draw_bars(float display_min_time_ms,
                    float bar_scale,
                    float bar_height,
                    float visible_min_time_ms,
                    float visible_max_time_ms) {
        auto is_visible = [&](Sample *s) {
            if (s->end_time_ms < visible_min_time_ms ||
                s->start_time_ms > visible_max_time_ms) {
                return false;
            }
            return true;
        };

        std::vector<Sample *> sample_to_draw[2]{};
        uint32_t cur_index = 0;
        for (auto &s : samples) {
            if (is_visible(s.get())) {
                sample_to_draw[cur_index].push_back(s.get());
            }
        }

        auto offset = ImGui::GetCursorScreenPos();
        auto draw = ImGui::GetWindowDrawList();
        float height_offset = 0.0f;
        auto is_window_hovered = ImGui::IsWindowHovered();
        auto mouse_pos = ImGui::GetMousePos();
        for (; !sample_to_draw[cur_index].empty(); cur_index = 1 - cur_index) {
            auto &buf = sample_to_draw[cur_index];
            auto &next_buf = sample_to_draw[1 - cur_index];
            next_buf.clear();

            for (auto s : buf) {
                auto bar_min_x =
                    (s->start_time_ms - display_min_time_ms) * bar_scale;
                auto bar_max_x =
                    (s->end_time_ms - display_min_time_ms) * bar_scale;
                auto bar_min = offset + ImVec2(bar_min_x, height_offset);
                auto bar_max =
                    offset + ImVec2(bar_max_x, height_offset + bar_height);
                auto bar_rect =
                    ImVec4(bar_min.x, bar_min.y, bar_max.x, bar_max.y);

                auto is_bar_hovered =
                    is_window_hovered && mouse_pos.x >= bar_min.x &&
                    mouse_pos.y >= bar_min.y && mouse_pos.x <= bar_max.x &&
                    mouse_pos.y <= bar_max.y;

                auto bar_color = s->color;
                // Tint color on hover
                if (is_bar_hovered) {
                    auto color_v = ImGui::ColorConvertU32ToFloat4(bar_color);
                    float tint = 0.2f;
                    color_v.x = color_v.x * (1.0f - tint) + tint;
                    color_v.y = color_v.y * (1.0f - tint) + tint;
                    color_v.z = color_v.z * (1.0f - tint) + tint;
                    bar_color = ImGui::GetColorU32(color_v);
                }

                draw->AddRectFilled(bar_min, bar_max, bar_color, 0);
                auto info_label = fmt::format(
                    "{}({:.3f}ms)", s->name, s->end_time_ms - s->start_time_ms);
                draw_clipped_text(draw, bar_rect, info_label.c_str());

                if (is_bar_hovered) {
                    ImGui::SetTooltip("%s\nFile: %s\nLine: %d\nFunction: %s",
                                      info_label.c_str(),
                                      s->file_name ? s->file_name : "<null>",
                                      s->line,
                                      s->function_name ? s->function_name
                                                       : "<null>");
                    draw->AddRect(bar_min, bar_max, 0xFF0000FF);
                }

                for (auto &child : s->children) {
                    next_buf.push_back(child.get());
                }
            }

            height_offset += bar_height;
        }
        return height_offset;
    }

    void on_gui(const std::deque<float> &frame_times_ms,
                float display_min_time_ms,
                float display_max_time_ms,
                ProfilerGuiState &state) {
        ImGui::Text("%s", name.c_str());
        ImGui::BeginChild(name.c_str(),
                          ImVec2(0, state.window_heights[index]),
                          true,
                          ImGuiWindowFlags_HorizontalScrollbar);

        display_bar_graph(
            frame_times_ms, display_min_time_ms, display_max_time_ms, state);

        ImGui::EndChild();
    }

    void clear() {
        samples.clear();
    }
};

struct Profiler {
    ProfilerGroup groups[MAX_PROFILER_GROUP_NUM]{};
    bool group_enabled[MAX_PROFILER_GROUP_NUM]{};
    TimePoint start_time{};
    bool pause = false;
    std::deque<float> frame_times_ms{};

    Profiler() {
        start_time = Clock::now();
        for (int i = 0; i < MAX_PROFILER_GROUP_NUM; i++) {
            groups[i].index = i;
        }
    }

    void set_pause(bool p) {
        pause = p;
        for (auto &g : groups) {
            g.pause = p;
        }
    }

    float get_time_ms() const {
        return std::chrono::duration<float, std::milli>(Clock::now() -
                                                        start_time)
            .count();
    }

    void begin_sample(size_t group_id,
                      const std::string &name,
                      uint32_t color,
                      float start_time_ms,
                      const char *file_name,
                      uint32_t line,
                      const char *function_name) {
        if (group_id < MAX_PROFILER_GROUP_NUM && group_enabled[group_id]) {
            groups[group_id].begin_sample(
                name, color, start_time_ms, file_name, line, function_name);
        }
    }

    void end_sample(size_t group_id, float end_time_ms) {
        if (group_id < MAX_PROFILER_GROUP_NUM && group_enabled[group_id]) {
            groups[group_id].end_sample(end_time_ms);
        }
    }

    void clear() {
        frame_times_ms.clear();
        for (auto &g : groups) {
            g.clear();
        }
    }

    void on_gui(ProfilerGuiState &state) {
        on_gui_controls(state);

        float display_min_time_ms, display_max_time_ms;
        {
            ARS_PROFILER_SAMPLE("Calculate Min Max", 0xFF131111);
            for (int i = 0; i < MAX_PROFILER_GROUP_NUM; i++) {
                if (group_enabled[i]) {
                    groups[i].sample_time_ms_min_max(display_min_time_ms,
                                                     display_max_time_ms);
                    break;
                }
            }
        }

        int id = 0;
        for (int i = 0; i < MAX_PROFILER_GROUP_NUM; i++) {
            if (group_enabled[i]) {
                ImGui::PushID(id++);
                groups[i].on_gui(frame_times_ms,
                                 display_min_time_ms,
                                 display_max_time_ms,
                                 state);
                ImGui::Button(
                    "##Resize",
                    ImVec2(ImGui::GetContentRegionAvailWidth(), 10.0));
                if (ImGui::IsItemActive()) {
                    auto delta = ImGui::GetIO().MouseDelta.y;
                    auto &height = state.window_heights[i];
                    height += delta;
                    height = std::max(30.0f, height);
                }
                ImGui::PopID();
            }
        }
    }

    float average_frame_time_ms() {
        if (frame_times_ms.size() <= 1) {
            return 0.0f;
        }
        auto gather_frame =
            std::min(PROFILER_AVERAGE_FPS_GATHER_FRAMES, frame_times_ms.size());
        auto start_time_ms =
            frame_times_ms[frame_times_ms.size() - gather_frame];
        auto end_time_ms = frame_times_ms.back();
        return (end_time_ms - start_time_ms) /
               static_cast<float>(gather_frame - 1);
    }

    void on_gui_controls(ProfilerGuiState &state) {
        if (ImGui::Checkbox("Pause", &pause)) {
            set_pause(pause);
        }
        ImGui::SameLine();
        if (ImGui::Button("Clear")) {
            clear();
        }
        ImGui::SameLine();
        ImGui::Text("Scale");
        ImGui::SameLine();
        auto scale_button_strength = 1.5f;
        if (ImGui::Button("+")) {
            state.bar_scale *= scale_button_strength;
        }
        ImGui::SameLine();
        if (ImGui::Button("-")) {
            state.bar_scale /= scale_button_strength;
        }
        ImGui::SameLine();
        auto ave_frame_time_ms = average_frame_time_ms();
        ImGui::Text("Average %.3f ms/frame (%.2f FPS)",
                    ave_frame_time_ms,
                    1000.0f / ave_frame_time_ms);

        ImGui::SliderFloat("Horizontal Scroll", &state.scroll_x, 0.0f, 1.0f);
    }

    void new_frame() {
        if (pause) {
            return;
        }
        frame_times_ms.push_back(get_time_ms());
        if (frame_times_ms.size() > MAX_PROFILER_SAMPLE_NUM) {
            frame_times_ms.pop_front();
        }
    }
};

std::unique_ptr<Profiler> s_profiler{};
} // namespace

void init_profiler() {
#ifdef ARS_PROFILER_ENABLED
    s_profiler = std::make_unique<Profiler>();
    profiler_enable_group(PROFILER_GROUP_CPU_MAIN_THREAD, true);
    profiler_enable_group(PROFILER_GROUP_GPU, true);

    profiler_set_group_name(PROFILER_GROUP_CPU_MAIN_THREAD,
                            PROFILER_GROUP_CPU_MAIN_THREAD_NAME);
    profiler_set_group_name(PROFILER_GROUP_GPU, PROFILER_GROUP_GPU_NAME);
#endif
}

void destroy_profiler() {
    s_profiler = nullptr;
}

void profiler_begin_sample(size_t group_id,
                           const std::string &name,
                           uint32_t color,
                           const char *file_name,
                           uint32_t line,
                           const char *function_name,
                           float start_time_ms) {
    if (s_profiler != nullptr) {
        s_profiler->begin_sample(group_id,
                                 name,
                                 color,
                                 start_time_ms,
                                 file_name,
                                 line,
                                 function_name);
    }
}

void profiler_end_sample(size_t group_id, float end_time_ms) {
    if (s_profiler != nullptr) {
        s_profiler->end_sample(group_id, end_time_ms);
    }
}

void profiler_begin_sample(size_t group_id,
                           const std::string &name,
                           uint32_t color,
                           const char *file_name,
                           uint32_t line,
                           const char *function_name) {
    if (s_profiler != nullptr) {
        profiler_begin_sample(group_id,
                              name,
                              color,
                              file_name,
                              line,
                              function_name,
                              s_profiler->get_time_ms());
    }
}

void profiler_end_sample(size_t group_id) {
    if (s_profiler != nullptr) {
        profiler_end_sample(group_id, s_profiler->get_time_ms());
    }
}

void profiler_on_gui(const std::string &window_name, ProfilerGuiState &state) {
    ARS_PROFILER_SAMPLE("Profiler Window", 0xFF125932);
    ImGui::Begin(window_name.c_str());
    if (s_profiler == nullptr) {
        ImGui::Text("Profiler Not Enabled");
    } else {
        s_profiler->on_gui(state);
    }
    ImGui::End();
}

void profiler_enable_group(size_t group_id, bool enable) {
    if (s_profiler != nullptr && group_id < MAX_PROFILER_GROUP_NUM) {
        s_profiler->group_enabled[group_id] = enable;
    }
}

bool profiler_group_enabled(size_t group_id) {
    if (s_profiler == nullptr || group_id >= MAX_PROFILER_GROUP_NUM) {
        return false;
    }
    return s_profiler->group_enabled[group_id];
}

std::string profiler_group_name(size_t group_id) {
    if (s_profiler == nullptr || group_id >= MAX_PROFILER_GROUP_NUM) {
        return "";
    }
    return s_profiler->groups[group_id].name;
}

void profiler_set_group_name(size_t group_id, const std::string &name) {
    if (s_profiler == nullptr || group_id >= MAX_PROFILER_GROUP_NUM) {
        return;
    }
    s_profiler->groups[group_id].name = name;
}

bool profiler_inited() {
    return s_profiler != nullptr;
}

float profiler_time_ms_from_inited() {
    if (s_profiler != nullptr) {
        return s_profiler->get_time_ms();
    }
    return 0.0f;
}

void profiler_new_frame() {
    if (s_profiler != nullptr) {
        s_profiler->new_frame();
    }
}

ProfilerGuiState::ProfilerGuiState() {
    for (auto &h : window_heights) {
        h = 100.0f;
    }
}
} // namespace ars