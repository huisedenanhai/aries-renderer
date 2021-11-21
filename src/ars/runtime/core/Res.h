#pragma once

#include <any>
#include <memory>
#include <rttr/variant.h>

namespace ars {
std::string canonical_res_path(const std::string &path);
bool starts_with(const std::string &str, const std::string &prefix);

// Resources path should all use '/' as separator
class ResHandle {
  public:
    ResHandle() {
        _handle = std::make_shared<Handle>();
    }

    std::string path() const {
        return _handle->path;
    }

    void set_path(const std::string &path) {
        _handle->path = path;
    }

    rttr::variant get_variant() const {
        return _handle->res;
    }

    void set_variant(const rttr::variant &res) const {
        _handle->res = res;
    }

  protected:
    struct Handle {
        std::string path{};
        // Should be std::shared_ptr<T> for the underlying resource type T
        rttr::variant res{};
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

class IDataProvider {
  public:
    virtual ~IDataProvider() = default;

    virtual std::any load(const std::string &path) = 0;
};

class IResLoader {
  public:
    virtual ResHandle load(std::any data) = 0;
};

class Resources {
  public:
    void mount(const std::string &path,
               const std::shared_ptr<IDataProvider> &provider);

    void register_res_loader(const rttr::type &ty,
                             const std::shared_ptr<IResLoader> &loader);

    template <typename T>
    void register_res_loader(const std::shared_ptr<IResLoader> &loader) {
        register_res_loader(rttr::type::get<T>(), loader);
    }

    ResHandle load(const rttr::type &ty, const std::string &path);

    template <typename T> Res<T> load(const std::string &path) {
        return Res<T>(load(rttr::type::get<T>(), path));
    }

  private:
    IDataProvider *resolve_path(const std::string &path,
                                std::string &relative_path);

    // Keys are canonical path of mount point
    std::unordered_map<std::string, std::shared_ptr<IDataProvider>>
        _data_providers;
    std::unordered_map<rttr::type, std::shared_ptr<IResLoader>> _res_loaders;
};
} // namespace ars