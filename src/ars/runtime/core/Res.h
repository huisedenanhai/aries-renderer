#pragma once

#include <ios>
#include <memory>
#include <nlohmann/json.hpp>
#include <rttr/variant.h>
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

template <typename T> class Res;

// Resources path should all use '/' as separator.
// For subresources, use ':' to separate names, names should not be empty
// string. e.g. '/Model.gltf:mesh_0'
class ResHandle {
  public:
    ResHandle();

    std::string path() const;
    void set_path(const std::string &path);

    rttr::variant get_variant() const;
    void set_variant(const rttr::variant &res) const;

    // The concept of sub res should be useful for composite resources like
    // models and sprite atlases.
    // One should be careful to not introduce circular reference.
    ResHandle get_sub_res(const std::string &name) const;
    void set_sub_res(const std::string &name, const ResHandle &res) const;

    template <typename T> Res<T> get_sub(const std::string &name) const {
        return Res<T>(get_sub_res(name));
    }

    template <typename T>
    void set_sub(const std::string &name, const Res<T> &res) const {
        set_sub_res(name, res);
    }

  protected:
    struct Handle {
        // The full path of the resources
        std::string path{};
        // Should be std::shared_ptr<T> for the underlying resource type T
        rttr::variant res{};
        std::unordered_map<std::string, ResHandle> sub_res{};
    };

    std::shared_ptr<Handle> _handle{};
};

template <typename T> class Res : public ResHandle {
  public:
    using ResHandle::ResHandle;

    explicit Res(const ResHandle &res) : ResHandle(res) {}

    std::shared_ptr<T> get() const {
        return _handle->res.template get_value<std::shared_ptr<T>>();
    }

    void set(const std::shared_ptr<T> &res) const {
        _handle->res = res;
    }

    T *operator->() const {
        return get().get();
    }
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
    virtual ResHandle load(ResData data) = 0;
};

class Resources {
  public:
    void mount(const std::string &path,
               const std::shared_ptr<IDataProvider> &provider);

    void register_res_loader(const std::string &ty,
                             const std::shared_ptr<IResLoader> &loader);

    template <typename T>
    void register_res_loader(const std::shared_ptr<IResLoader> &loader) {
        register_res_loader(rttr::type::get<T>(), loader);
    }

    ResHandle load_res(const std::string &path);

    template <typename T> Res<T> load(const std::string &path) {
        return Res<T>(load_res(path));
    }

  private:
    ResHandle load_root_res(const std::string &path);

    IDataProvider *resolve_path(const std::string &path,
                                std::string &relative_path);

    // Keys are canonical path of mount point
    std::unordered_map<std::string, std::shared_ptr<IDataProvider>>
        _data_providers;
    std::unordered_map<std::string, std::shared_ptr<IResLoader>> _res_loaders;
};
} // namespace ars