#pragma once

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
  public:
    virtual ~IRes() = default;

    [[nodiscard]] std::string path() const;
    void set_path(const std::string &path);

  private:
    // The full path of the resources
    std::string _path{};
};
} // namespace ars