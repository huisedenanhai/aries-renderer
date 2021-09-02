#pragma once

#include <string>

namespace ars {
void log_info(const std::string &msg);
void log_warn(const std::string &msg);
void log_error(const std::string &msg);
void panic(const std::string &msg);
} // namespace ars