#pragma once

#include <rttr/rttr_enable.h>
#include <string>

// Inherit IRes to implement resources
//
// This header is included in some important headers which are included nearly
// in all sources (like ITexture.h and IMesh.h).
// DO NOT put or include any heavy metaprogramming headers here (like
// nlohmann/json.hpp)
namespace ars {
// Resources path should all use '/' as separator.
class IRes {
    RTTR_ENABLE();

  public:
    virtual ~IRes() = default;

    [[nodiscard]] std::string path() const;
    void set_path(const std::string &path);

//    virtual std::string res_type() const = 0;

    static void register_type();

  private:
    // The full path of the resources
    std::string _path{};
};
} // namespace ars