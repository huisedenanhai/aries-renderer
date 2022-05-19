#include "Profiler.h"
#include "Context.h"
#include <ars/runtime/core/Log.h>

namespace ars::render::vk {
constexpr uint32_t MAX_PROFILER_QUERY_COUNT_VK = 4000;

Profiler::Profiler(Context *context)
    : _context(context),
      _time_stamp_period_ns(context->info().properties.limits.timestampPeriod) {
    VkQueryPoolCreateInfo info{VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO};
    info.queryCount = MAX_PROFILER_QUERY_COUNT_VK;
    info.queryType = VK_QUERY_TYPE_TIMESTAMP;

    auto device = _context->device();
    if (device->Create(&info, &_query_pool) != VK_SUCCESS) {
        ARS_LOG_CRITICAL("Failed to create profiler query pool");
        return;
    }

    _context->queue()->submit_once([&](CommandBuffer *cmd) {
        cmd->ResetQueryPool(_query_pool, 0, MAX_PROFILER_QUERY_COUNT_VK);
    });

    _time_stamp_mask >>= std::max(
        0u, (64 - context->queue()->family_properties().timestampValidBits));
}

Profiler::~Profiler() {
    if (_query_pool != VK_NULL_HANDLE) {
        _context->device()->Destroy(_query_pool);
    }
}

void Profiler::begin_sample(CommandBuffer *cmd,
                            const std::string &name,
                            uint32_t color,
                            const char *file_name,
                            uint32_t line,
                            const char *function_name) {
    auto query_cmd = add_query_command(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    if (query_cmd != nullptr) {
        query_cmd->type = QueryCommandType::BeginSample;
        query_cmd->name = name;
        query_cmd->color = color;
        query_cmd->file_name = file_name;
        query_cmd->line = line;
        query_cmd->function_name = function_name;
    }
}

void Profiler::end_sample(CommandBuffer *cmd) {
    auto query_cmd =
        add_query_command(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
    if (query_cmd != nullptr) {
        query_cmd->type = QueryCommandType::EndSample;
    }
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
    float frame_start_time_ms = 0.0f;
    uint64_t frame_start_ts = 0;

    auto get_time_ms = [&](uint64_t ts) {
        return delta_time_ms(frame_start_ts, ts) + frame_start_time_ms;
    };
    for (int i = 0; i < _commands.size(); i++) {
        auto &cmd = _commands[i];
        auto ts = time_stamps[i];
        auto time_ms = get_time_ms(ts);
        switch (cmd.type) {
        case QueryCommandType::BeginFrame:
            // Correct time stamp on every frame begin.
            // When there is no workload on GPU the time stamp may not change,
            // thus the global time can not be inferred from timestamp.
            frame_start_time_ms = cmd.frame_start_time_ms;
            frame_start_ts = ts;
            break;
        case QueryCommandType::BeginSample:
            ars::profiler_begin_sample(PROFILER_GROUP_GPU,
                                       cmd.name,
                                       cmd.color,
                                       cmd.file_name,
                                       cmd.line,
                                       cmd.function_name,
                                       time_ms);
            pending_count++;
            break;
        case QueryCommandType::EndSample:
            ars::profiler_end_sample(PROFILER_GROUP_GPU, time_ms);
            pending_count--;
            break;
        }
    }

    // Fix unpaired samples due to fixed query pool size
    auto end_time_ms = get_time_ms(time_stamps.back());
    for (int i = 0; i < pending_count; i++) {
        ars::profiler_end_sample(PROFILER_GROUP_GPU, end_time_ms);
    }
}

void Profiler::begin_frame() {
    _context->queue()->submit_once([&](CommandBuffer *cmd) {
        auto query_cmd =
            add_query_command(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
        if (query_cmd != nullptr) {
            query_cmd->type = QueryCommandType::BeginFrame;
            query_cmd->frame_start_time_ms =
                ars::profiler_time_ms_from_inited();
        }
    });
}

float Profiler::delta_time_ms(uint64_t ts_from, uint64_t ts_to) const {
    return static_cast<float>((double(ts_to & _time_stamp_mask) -
                               double(ts_from & _time_stamp_mask)) *
                              _time_stamp_period_ns * 1e-6);
}

Profiler::QueryCommand *
Profiler::add_query_command(CommandBuffer *cmd, VkPipelineStageFlagBits stage) {
    if (_commands.size() > MAX_PROFILER_QUERY_COUNT_VK) {
        ARS_LOG_ERROR(
            "Profiler sample count exceeds MAX_PROFILER_QUERY_COUNT_VK = {}",
            MAX_PROFILER_QUERY_COUNT_VK);
        return nullptr;
    }

    cmd->WriteTimestamp(stage, _query_pool, _commands.size());
    _commands.emplace_back();
    return &_commands.back();
}
} // namespace ars::render::vk