#pragma once

#include <ios>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>

namespace ars {
std::string canonical_res_path(const std::string &path);
bool starts_with(const std::string &str, const std::string &prefix);
std::vector<std::string> split_by(const std::string &str,
                                  const std::string &sep);

template <typename It>
std::string join(It &&beg, It &&end, const std::string &sep) {
    std::ostringstream ss{};
    bool first = true;
    for (; beg != end; beg++) {
        if (first) {
            first = false;
        } else {
            ss << sep;
        }
        ss << *beg;
    }
    return ss.str();
}

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

struct ResData {
    // Use std::string rather than rttr::type or other similar stuff because:
    // 1. All we want here is an id.
    // 2. rttr::type::get_by_name() access global registry, looks unnecessary
    // for here and seems not thread safe.
    std::string ty{};
    nlohmann::json meta{};
    std::vector<uint8_t> data{};

    void reset();

    bool valid() const;

    // 4 byte magic
    static constexpr uint8_t MAGIC_NUMBER[4] = {0xAD, 0x92, 0x77, 0xBA};

    std::ostream &serialize(std::ostream &os) const;
    std::istream &deserialize(std::istream &is);

    void save(const std::filesystem::path &path) const;
    void load(const std::filesystem::path &path);
};

// Add preferred extension '.ares' to the path
std::filesystem::path preferred_res_path(const std::filesystem::path &path);

struct DataSlice {
    uint64_t offset = 0;
    uint64_t size = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(DataSlice, offset, size)
};

class IDataProvider {
  public:
    virtual ~IDataProvider() = default;

    virtual ResData load(const std::string &path) = 0;
};

class IResLoader {
  public:
    virtual std::shared_ptr<IRes> load(ResData data) = 0;
};

class Resources {
  public:
    void mount(const std::string &path,
               const std::shared_ptr<IDataProvider> &provider);

    void register_res_loader(const std::string &ty,
                             const std::shared_ptr<IResLoader> &loader);

    std::shared_ptr<IRes> load_res(const std::string &path);

    template <typename T> std::shared_ptr<T> load(const std::string &path) {
        return std::dynamic_pointer_cast<T>(load_res(path));
    }

  private:
    std::shared_ptr<IRes> load_root_res(const std::string &path);

    IDataProvider *resolve_path(const std::string &path,
                                std::string &relative_path);

    // Keys are canonical path of mount point
    std::unordered_map<std::string, std::shared_ptr<IDataProvider>>
        _data_providers;
    std::unordered_map<std::string, std::shared_ptr<IResLoader>> _res_loaders;
};
} // namespace ars