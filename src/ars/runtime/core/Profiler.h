#pragma once

#include "misc/Defer.h"
#include <string>

// If ARS_PROFILER_ENABLED if not defined, all profiler related macros should be
// no op, and profiler related methods should take as less runtime overhead as
// possible.
#define ARS_PROFILER_ENABLED

namespace ars {
constexpr size_t MAX_PROFILER_GROUP_NUM = 64;
constexpr size_t MAX_PROFILER_SAMPLE_NUM = 512;

// These groups are enabled by default
constexpr size_t PROFILER_GROUP_CPU_MAIN_THREAD = 0;
constexpr size_t PROFILER_GROUP_GPU = 1;

// These names are set by default
constexpr const char *PROFILER_GROUP_CPU_MAIN_THREAD_NAME = "Main";
constexpr const char *PROFILER_GROUP_GPU_NAME = "GPU";

void init_profiler();
void destroy_profiler();

bool profiler_inited();

void profiler_enable_group(size_t group_id, bool enable);
bool profiler_group_enabled(size_t group_id);

std::string profiler_group_name(size_t group_id);
void profiler_set_group_name(size_t group_id, const std::string &name);

void profiler_begin_sample(size_t group_id,
                           const std::string &name,
                           uint32_t color);
void profiler_end_sample(size_t group_id);

void profiler_begin_sample(size_t group_id,
                           const std::string &name,
                           uint32_t color,
                           float start_time_ms);
void profiler_end_sample(size_t group_id, float end_time_ms);

struct ProfilerGuiState {
    float bar_scale = 30.0f;
    float max_horizontal_scroll = 5000.0f;
    float horizontal_scroll_ratio = 1.0f;
};

void profiler_on_gui(const std::string &window_name, ProfilerGuiState &state);

} // namespace ars

#ifdef ARS_PROFILER_ENABLED
#define ARS_PROFILER_SAMPLE(name, color)                                       \
    ars::profiler_begin_sample(                                                \
        PROFILER_GROUP_CPU_MAIN_THREAD, (name), (color));                      \
    ARS_DEFER_TAGGED(profiler_sample, [&]() {                                  \
        ars::profiler_end_sample(PROFILER_GROUP_CPU_MAIN_THREAD);              \
    })
#else
#define ARS_PROFILER_SAMPLE(name, color)
#endif