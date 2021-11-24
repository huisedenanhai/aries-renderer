#pragma once

#include <string>
#include <unordered_map>

// README: inherit IRes to implement resources
// This header is included in some important headers which are included nearly
// in all sources (like ITexture.h and IMesh.h).
// DO NOT put or include any heavy metaprogramming headers here (like
// nlohmann/json.hpp)
namespace ars {
// Resources path should all use '/' as separator.
// For subresources, use ':' to separate names, names should not be empty
// string. e.g. '/Model.gltf:mesh_0'
class IRes {
  public:
    virtual ~IRes() = default;

    [[nodiscard]] std::string path() const;
    void set_path(const std::string &path);

    [[nodiscard]] std::shared_ptr<IRes>
    get_sub_res(const std::string &name) const;
    void set_sub_res(const std::string &name, const std::shared_ptr<IRes> &res);

  private:
    // The full path of the resources
    std::string _path{};
    std::unordered_map<std::string, std::shared_ptr<IRes>> _sub_res{};
};
} // namespace ars