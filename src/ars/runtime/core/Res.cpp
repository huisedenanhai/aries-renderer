#include "Res.h"
#include "Log.h"
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

bool starts_with(const std::string &str, const std::string &prefix) {
    if (prefix.size() > str.size()) {
        return false;
    }
    auto prefix_size = prefix.size();
    for (int i = 0; i < prefix_size; i++) {
        if (str[i] != prefix[i]) {
            return false;
        }
    }
    return true;
}

ResHandle Resources::load(const rttr::type &ty, const std::string &path) {
    std::string relative_path{};
    auto provider = resolve_path(path, relative_path);
    if (provider == nullptr) {
        ARS_LOG_ERROR("Failed to load resources {}: No data provider found.",
                      path);
        return {};
    }

    auto loader_it = _res_loaders.find(ty);
    if (loader_it == _res_loaders.end()) {
        ARS_LOG_ERROR(
            "Failed to load resources {}: No loader registered for type {}.",
            path,
            ty.get_name().to_string());
        return {};
    }
    auto data = provider->load(relative_path);
    if (data.has_value()) {
        ARS_LOG_ERROR("Failed to load resources {}: Failed to load data.",
                      path);
        return {};
    }

    return loader_it->second->load(std::move(data));
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
    auto canonical_path = canonical_res_path(path);
    std::string root{};
    IDataProvider *provider = nullptr;
    for (auto &[r, p] : _data_providers) {
        if (!starts_with(canonical_path, r)) {
            continue;
        }
        // Find the longest prefix
        if (provider == nullptr || root.size() < r.size()) {
            root = r;
            provider = p.get();
        }
    }

    // Does not touch relative path output if no match is found
    if (provider == nullptr) {
        return nullptr;
    }

    relative_path = canonical_path.substr(root.size());
    return provider;
}
} // namespace ars