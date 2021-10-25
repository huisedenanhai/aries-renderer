#pragma once

#include <spdlog/spdlog.h>

#undef SPDLOG_LOGGER_TRACE
#undef SPDLOG_LOGGER_DEBUG
#undef SPDLOG_LOGGER_INFO
#undef SPDLOG_LOGGER_WARN
#undef SPDLOG_LOGGER_ERROR
#undef SPDLOG_LOGGER_CRITICAL

#undef SPDLOG_TRACE
#undef SPDLOG_DEBUG
#undef SPDLOG_INFO
#undef SPDLOG_WARN
#undef SPDLOG_ERROR
#undef SPDLOG_CRITICAL

#define ARS_LOG_LOGGER_CALL(logger, level, ...)                                \
    SPDLOG_LOGGER_CALL(logger, level, __VA_ARGS__)

#if ARS_LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_TRACE
#define ARS_LOG_LOGGER_TRACE(logger, ...)                                      \
    ARS_LOG_LOGGER_CALL(logger, spdlog::level::trace, __VA_ARGS__)
#define ARS_LOG_TRACE(...)                                                     \
    ARS_LOG_LOGGER_TRACE(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define ARS_LOG_LOGGER_TRACE(logger, ...) (void)0
#define ARS_LOG_TRACE(...) (void)0
#endif

#if ARS_LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_DEBUG
#define ARS_LOG_LOGGER_DEBUG(logger, ...)                                      \
    ARS_LOG_LOGGER_CALL(logger, spdlog::level::debug, __VA_ARGS__)
#define ARS_LOG_DEBUG(...)                                                     \
    ARS_LOG_LOGGER_DEBUG(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define ARS_LOG_LOGGER_DEBUG(logger, ...) (void)0
#define ARS_LOG_DEBUG(...) (void)0
#endif

#if ARS_LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_INFO
#define ARS_LOG_LOGGER_INFO(logger, ...)                                       \
    ARS_LOG_LOGGER_CALL(logger, spdlog::level::info, __VA_ARGS__)
#define ARS_LOG_INFO(...)                                                      \
    ARS_LOG_LOGGER_INFO(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define ARS_LOG_LOGGER_INFO(logger, ...) (void)0
#define ARS_LOG_INFO(...) (void)0
#endif

#if ARS_LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_WARN
#define ARS_LOG_LOGGER_WARN(logger, ...)                                       \
    ARS_LOG_LOGGER_CALL(logger, spdlog::level::warn, __VA_ARGS__)
#define ARS_LOG_WARN(...)                                                      \
    ARS_LOG_LOGGER_WARN(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define ARS_LOG_LOGGER_WARN(logger, ...) (void)0
#define ARS_LOG_WARN(...) (void)0
#endif

#if ARS_LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_ERROR
#define ARS_LOG_LOGGER_ERROR(logger, ...)                                      \
    ARS_LOG_LOGGER_CALL(logger, spdlog::level::err, __VA_ARGS__)
#define ARS_LOG_ERROR(...)                                                     \
    ARS_LOG_LOGGER_ERROR(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define ARS_LOG_LOGGER_ERROR(logger, ...) (void)0
#define ARS_LOG_ERROR(...) (void)0
#endif

#if ARS_LOG_ACTIVE_LEVEL <= SPDLOG_LEVEL_CRITICAL
#define ARS_LOG_LOGGER_CRITICAL(logger, ...)                                   \
    ARS_LOG_LOGGER_CALL(logger, spdlog::level::critical, __VA_ARGS__)
#define ARS_LOG_CRITICAL(...)                                                  \
    ARS_LOG_LOGGER_CRITICAL(spdlog::default_logger_raw(), __VA_ARGS__)
#else
#define ARS_LOG_LOGGER_CRITICAL(logger, ...) (void)0
#define ARS_LOG_CRITICAL(...) (void)0
#endif
