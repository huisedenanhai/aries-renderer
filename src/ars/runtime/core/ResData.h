#pragma once

#include "Res.h"
#include <any>
#include <ios>
#include <memory>
#include <nlohmann/json.hpp>
#include <sstream>

namespace ars {
std::string canonical_res_path(const std::string &path);
std::string canonical_res_path(const std::filesystem::path &path);
std::string canonical_res_path(const char *path);
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

    // The path will not start with '/'
    // Need to do necessary modification to the path like automatically append
    // .ares extension
    virtual ResData load(const std::string &path) = 0;
};

class FolderDataProvider : public IDataProvider {
  public:
    explicit FolderDataProvider(std::filesystem::path root);
    ResData load(const std::string &path) override;

  private:
    std::filesystem::path _root{};
};

class IResLoader {
  public:
    virtual std::shared_ptr<IRes> load(const ResData &data) = 0;
};

template <typename Func> auto make_loader(Func &&func) {
    struct Loader : public IResLoader {
      public:
        explicit Loader(Func &&f) : f(f) {}

        std::shared_ptr<IRes> load(const ResData &data) override {
            return f(data);
        }

        Func f;
    };

    return std::make_shared<Loader>(std::forward<Func>(func));
}

class Resources {
  public:
    void mount(const std::string &path,
               const std::shared_ptr<IDataProvider> &provider);

    void register_res_loader(const std::string &ty,
                             const std::shared_ptr<IResLoader> &loader);

    // Should not append .ares extension in path
    std::shared_ptr<IRes> load_res(const std::string &path);

    template <typename T> std::shared_ptr<T> load(const std::string &path) {
        return std::dynamic_pointer_cast<T>(load_res(path));
    }

  private:
    std::shared_ptr<IRes> load_res_no_cache(const std::string &path);
    IDataProvider *resolve_path(const std::string &path,
                                std::string &relative_path);

    std::unordered_map<std::string, std::shared_ptr<IRes>> _cache{};
    // Keys are canonical path of mount point
    std::unordered_map<std::string, std::shared_ptr<IDataProvider>>
        _data_providers;
    std::unordered_map<std::string, std::shared_ptr<IResLoader>> _res_loaders;
};
} // namespace ars