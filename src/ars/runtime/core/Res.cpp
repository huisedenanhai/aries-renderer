#include "Res.h"
#include "misc/Visitor.h"

namespace ars {
std::string canonical_res_path(const std::string &path) {
    // include trailing zero and possible prefix '/'
    auto cs = std::make_unique<char[]>(path.size() + 3);
    cs[0] = '/';
    int i = 1;
    for (auto c : path) {
        if (cs[i - 1] == '/' && c == '/') {
            continue;
        }
        cs[i++] = c;
    }
    // Remove last '/'
    if (i > 1 && cs[i - 1] == '/') {
        i--;
    }
    cs[i] = 0;
    return {cs.get()};
}

ResHandle Resources::load(const rttr::type &ty, const std::string &url) {
    return {};
}

void Resources::mount(const std::string &path,
                      const std::shared_ptr<IDataProvider> &provider) {
    _data_providers[canonical_res_path(path)] = provider;
}

void Resources::register_res_loader(const rttr::type &ty,
                                    const std::shared_ptr<IResLoader> &loader) {
    _res_loaders[ty] = loader;
}

IDataProvider *Resources::resolve_path(const std::string &path,
                                       std::string &relative_path) {
    return nullptr;
}
} // namespace ars