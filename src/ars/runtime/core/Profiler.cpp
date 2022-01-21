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
};

struct ProfilerGroup {
    std::string name;
    std::deque<std::unique_ptr<Sample>> samples{};
    std::stack<std::unique_ptr<Sample>> working_stack{};
    bool pause = false;

    void begin_sample(const std::string &sample_name,
                      uint32_t color,
                      float start_time_ms) {
        auto s = std::make_unique<Sample>();
        s->name = sample_name;
        s->color = color;
        s->start_time_ms = start_time_ms;

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

    void on_gui(float cur_time_ms, ProfilerGuiState &state) {
        ImGui::Text("%s", name.c_str());
        auto offset = ImGui::GetCursorScreenPos();
        auto width = ImGui::GetContentRegionAvailWidth();
        auto draw = ImGui::GetWindowDrawList();

        float height_offset = 0.0f;
        float bar_height = ImGui::GetTextLineHeightWithSpacing();
        auto bar_scale = state.bar_scale;

        float display_start_time_ms = cur_time_ms - width / bar_scale;
        float display_end_time_ms = cur_time_ms;

        auto sample_is_visible = [&](Sample *s) {
            if (s->end_time_ms < display_start_time_ms ||
                s->start_time_ms > display_end_time_ms) {
                return false;
            }
            return true;
        };

        std::vector<Sample *> sample_to_draw[2]{};
        uint32_t cur_index = 0;

        for (auto &s : samples) {
            if (sample_is_visible(s.get())) {
                sample_to_draw[cur_index].push_back(s.get());
            }
        }

        for (; !sample_to_draw[cur_index].empty(); cur_index = 1 - cur_index) {
            auto &buf = sample_to_draw[cur_index];
            auto &next_buf = sample_to_draw[1 - cur_index];
            next_buf.clear();

            for (auto s : buf) {
                auto bar_min_x =
                    (s->start_time_ms - display_start_time_ms) * bar_scale;
                auto bar_max_x =
                    (s->end_time_ms - display_start_time_ms) * bar_scale;
                auto bar_min = offset + ImVec2(bar_min_x, height_offset);
                auto bar_max =
                    offset + ImVec2(bar_max_x, height_offset + bar_height);
                auto bar_rect =
                    ImVec4(bar_min.x, bar_min.y, bar_max.x, bar_max.y);

                draw->AddRectFilled(bar_min, bar_max, s->color, 0);
                auto info_label = fmt::format(
                    "{}({:.3f}ms)", s->name, s->end_time_ms - s->start_time_ms);
                draw_clipped_text(draw, bar_rect, info_label.c_str());

                for (auto &child : s->children) {
                    next_buf.push_back(child.get());
                }
            }

            height_offset += bar_height;
        }
        ImGui::Dummy(ImVec2(width, height_offset));
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
    float pause_start_time_ms = 0.0f;

    Profiler() {
        start_time = Clock::now();
        groups[PROFILER_GROUP_CPU_MAIN_THREAD].name =
            PROFILER_GROUP_CPU_MAIN_THREAD_NAME;
        groups[PROFILER_GROUP_GPU].name = PROFILER_GROUP_GPU_NAME;
    }

    void set_pause(bool p) {
        pause = p;
        for (auto &g : groups) {
            g.pause = p;
        }
        if (p) {
            pause_start_time_ms = get_time_ms();
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
                      float start_time_ms) {
        if (group_id < MAX_PROFILER_GROUP_NUM && group_enabled[group_id]) {
            groups[group_id].begin_sample(name, color, start_time_ms);
        }
    }

    void end_sample(size_t group_id, float end_time_ms) {
        if (group_id < MAX_PROFILER_GROUP_NUM && group_enabled[group_id]) {
            groups[group_id].end_sample(end_time_ms);
        }
    }

    void clear() {
        for (auto &g : groups) {
            g.clear();
        }
    }

    void on_gui(ProfilerGuiState &state) {
        on_gui_controls(state);

        auto cur_time_ms = pause ? pause_start_time_ms : get_time_ms();
        cur_time_ms -= state.max_horizontal_scroll *
                       (1.0f - state.horizontal_scroll_ratio) / state.bar_scale;
        for (int i = 0; i < MAX_PROFILER_GROUP_NUM; i++) {
            if (group_enabled[i]) {
                groups[i].on_gui(cur_time_ms, state);
            }
        }
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
        ImGui::SliderFloat(
            "Horizontal Scroll", &state.horizontal_scroll_ratio, 0.0f, 1.0f);
    }
};

std::unique_ptr<Profiler> s_profiler{};
} // namespace

void init_profiler() {
    s_profiler = std::make_unique<Profiler>();
}

void destroy_profiler() {
    s_profiler = nullptr;
}

void profiler_begin_sample(size_t group_id,
                           const std::string &name,
                           uint32_t color,
                           float start_time_ms) {
    if (s_profiler != nullptr) {
        s_profiler->begin_sample(group_id, name, color, start_time_ms);
    }
}

void profiler_end_sample(size_t group_id, float end_time_ms) {
    if (s_profiler != nullptr) {
        s_profiler->end_sample(group_id, end_time_ms);
    }
}

void profiler_begin_sample(size_t group_id,
                           const std::string &name,
                           uint32_t color) {
    if (s_profiler != nullptr) {
        profiler_begin_sample(group_id, name, color, s_profiler->get_time_ms());
    }
}

void profiler_end_sample(size_t group_id) {
    if (s_profiler != nullptr) {
        profiler_end_sample(group_id, s_profiler->get_time_ms());
    }
}

void profiler_on_gui(const std::string &window_name, ProfilerGuiState &state) {
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

} // namespace ars