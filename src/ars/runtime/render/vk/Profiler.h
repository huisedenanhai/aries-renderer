#pragma once

#include "Vulkan.h"
#include <ars/runtime/core/Profiler.h>
#include <vector>

namespace ars::render::vk {
class Profiler {
  public:
    explicit Profiler(Context *context);
    ~Profiler();

    void
    begin_sample(CommandBuffer *cmd, const std::string &name, uint32_t color);
    void end_sample(CommandBuffer *cmd);
    void begin_frame();

    void flush();

  private:
    float delta_time_ms(uint64_t ts_from, uint64_t ts_to) const;

    Context *_context = nullptr;
    VkQueryPool _query_pool = VK_NULL_HANDLE;
    float _time_stamp_period_ns = 0.0f;
    uint64_t _time_stamp_mask = ~0;

    enum class QueryCommandType {
        BeginFrame,
        BeginSample,
        EndSample,
    };

    struct QueryCommand {
        QueryCommandType type{};
        // name and color are only valid when query command type is BeginSample
        std::string name{};
        uint32_t color{};
        // frame_start_time_ms only valid when query command type is BeginFrame
        float frame_start_time_ms{};
    };

    std::vector<QueryCommand> _commands{};

    QueryCommand *add_query_command(CommandBuffer *cmd,
                                    VkPipelineStageFlagBits stage);
};
} // namespace ars::render::vk

#ifdef ARS_PROFILER_ENABLED
#define ARS_PROFILER_SAMPLE_VK_ONLY(cmd, name, color)                          \
    (cmd)->begin_sample(name, color);                                          \
    ARS_DEFER_TAGGED(vk_sample, [&]() { (cmd)->end_sample(); })
#else
#define ARS_PROFILER_SAMPLE_VK_ONLY(cmd, name, color)
#endif

#define ARS_PROFILER_SAMPLE_VK(cmd, name, color)                               \
    ARS_PROFILER_SAMPLE(name, color);                                          \
    ARS_PROFILER_SAMPLE_VK_ONLY(cmd, name, color)
