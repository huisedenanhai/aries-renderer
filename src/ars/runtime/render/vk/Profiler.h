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

    void flush();

  private:
    float time_ms(uint64_t time_stamp) const;

    Context *_context = nullptr;
    VkQueryPool _query_pool = VK_NULL_HANDLE;
    float _time_stamp_period_ns = 0.0f;
    uint64_t _start_time_stamp{};

    enum class QueryCommandType {
        BeginSample,
        EndSample,
    };

    struct QueryCommand {
        QueryCommandType type;
        // name and color are only valid when query command type if BeginSample
        std::string name;
        uint32_t color;
    };

    std::vector<QueryCommand> _commands{};
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
