#include "Log.h"
#include <iostream>
#include <stdexcept>

namespace ars {
void log_info(const std::string &msg) {
    std::cout << "[info]: " << msg << std::endl;
}

void log_warn(const std::string &msg) {
    std::cout << "[warn]: " << msg << std::endl;
}

void log_error(const std::string &msg) {
    std::cerr << "[error]: " << msg << std::endl;
}

void panic(const std::string &msg) {
    std::cerr << "[panic]: " << msg << std::endl;
    throw std::runtime_error(msg);
}
} // namespace ars