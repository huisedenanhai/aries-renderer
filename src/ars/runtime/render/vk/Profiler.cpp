#include "Profiler.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
constexpr uint32_t MAX_PROFILER_QUERY_COUNT_VK = 4000;

Profiler::Profiler(Context *context)
    : _context(context),
      _time_stamp_period_ns(context->properties().time_stamp_period_ns) {
    VkQueryPoolCreateInfo info{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
    info.queryCount = MAX_PROFILER_QUERY_COUNT_VK;
    info.queryType = VK_QUERY_TYPE_TIMESTAMP;

    auto device = _context->device();
    if (device->Create(&info, &_query_pool) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create profiler query pool");
        return;
    }

    _context->queue()->submit_once([&](CommandBuffer *cmd) {
        cmd->ResetQueryPool(_query_pool, 0, 1);
        cmd->WriteTimestamp(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, _query_pool, 0);
    });

    device->GetQueryPoolResults(_query_pool,
                                0,
                                1,
                                sizeof(uint64_t),
                                &_start_time_stamp,
                                0,
                                VK_QUERY_RESULT_64_BIT |
                                    VK_QUERY_RESULT_WAIT_BIT);

    _context->queue()->submit_once([&](CommandBuffer *cmd) {
        cmd->ResetQueryPool(_query_pool, 0, MAX_PROFILER_QUERY_COUNT_VK);
    });
}

Profiler::~Profiler() {
    if (_query_pool != VK_NULL_HANDLE) {
        _context->device()->Destroy(_query_pool);
    }
}

void Profiler::begin_sample(CommandBuffer *cmd,
                            const std::string &name,
                            uint32_t color) {
    if (_commands.size() > MAX_PROFILER_QUERY_COUNT_VK) {
        ARS_LOG_ERROR(
            "Profiler sample count exceeds MAX_PROFILER_QUERY_COUNT_VK = {}",
            MAX_PROFILER_QUERY_COUNT_VK);
        return;
    }

    cmd->WriteTimestamp(
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, _query_pool, _commands.size());
    QueryCommand query_cmd{};
    query_cmd.type = QueryCommandType::BeginSample;
    query_cmd.name = name;
    query_cmd.color = color;
    _commands.push_back(query_cmd);
}

void Profiler::end_sample(CommandBuffer *cmd) {
    if (_commands.size() > MAX_PROFILER_QUERY_COUNT_VK) {
        ARS_LOG_ERROR(
            "Profiler sample count exceeds MAX_PROFILER_QUERY_COUNT_VK = {}",
            MAX_PROFILER_QUERY_COUNT_VK);
        return;
    }

    cmd->WriteTimestamp(
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, _query_pool, _commands.size());
    QueryCommand query_cmd{};
    query_cmd.type = QueryCommandType::EndSample;
    _commands.push_back(query_cmd);
}

void Profiler::flush() {
    if (_commands.empty()) {
        return;
    }

    std::vector<uint64_t> time_stamps{};
    time_stamps.resize(_commands.size());

    auto device = _context->device();
    device->GetQueryPoolResults(_query_pool,
                                0,
                                _commands.size(),
                                sizeof(uint64_t) * time_stamps.size(),
                                time_stamps.data(),
                                sizeof(uint64_t),
                                VK_QUERY_RESULT_64_BIT |
                                    VK_QUERY_RESULT_WAIT_BIT);
    ARS_DEFER([&]() {
        _context->queue()->submit_once([&](CommandBuffer *cmd) {
            cmd->ResetQueryPool(_query_pool, 0, _commands.size());
        });
        _commands.clear();
    });

    int pending_count = 0;
    for (int i = 0; i < _commands.size(); i++) {
        auto &cmd = _commands[i];
        auto ts = time_stamps[i];
        switch (cmd.type) {
        case QueryCommandType::BeginSample:
            ars::profiler_begin_sample(
                PROFILER_GROUP_GPU, cmd.name, cmd.color, time_ms(ts));
            pending_count++;
            break;
        case QueryCommandType::EndSample:
            ars::profiler_end_sample(PROFILER_GROUP_GPU, time_ms(ts));
            pending_count--;
            break;
        }
    }

    // Fix unpaired samples due to fixed query pool size
    for (int i = 0; i < pending_count; i++) {
        ars::profiler_end_sample(PROFILER_GROUP_GPU,
                                 time_ms(time_stamps.back()));
    }
}

float Profiler::time_ms(uint64_t time_stamp) const {
    return static_cast<float>((double(time_stamp) - double(_start_time_stamp)) *
                              _time_stamp_period_ns * 1e-6);
}

} // namespace ars::render::vk