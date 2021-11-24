#include "Res.h"
#include "Log.h"
#include "misc/Visitor.h"
#include <fstream>

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

std::vector<std::string> split_by(const std::string &str,
                                  const std::string &sep) {
    int offset = 0;
    std::vector<std::string> res{};
    auto len = str.size();
    auto sep_len = sep.size();

    while (offset < len) {
        ARS_LOG_INFO("Sub {}", str.substr(offset));
        auto p = str.find(sep, offset);
        if (p != std::string::npos) {
            res.push_back(str.substr(offset, p - offset));
            offset = static_cast<int>(p + sep_len);
            // The tail of str happens to be the separator
            if (offset == len) {
                res.emplace_back("");
            }
        } else {
            res.push_back(str.substr(offset));
            break;
        }
    }

    return res;
}

std::shared_ptr<IRes> Resources::load_res(const std::string &path) {
    auto names = split_by(path, ":");
    names.erase(std::remove_if(names.begin(),
                               names.end(),
                               [&](std::string &s) { return s.empty(); }),
                names.end());

    if (names.empty()) {
        ARS_LOG_ERROR("Failed to load resources {}: Empty path.", path);
        return {};
    }

    auto res = load_root_res(names[0]);

    for (int i = 1; i < names.size(); i++) {
        res = res->get_sub_res(names[i]);
    }

    return res;
}

void Resources::mount(const std::string &path,
                      const std::shared_ptr<IDataProvider> &provider) {
    _data_providers[canonical_res_path(path)] = provider;
}

void Resources::register_res_loader(const std::string &ty,
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

std::shared_ptr<IRes> Resources::load_root_res(const std::string &path) {
    using namespace std::chrono;

    std::string relative_path{};
    auto provider = resolve_path(path, relative_path);
    if (provider == nullptr) {
        ARS_LOG_ERROR("Failed to load resources {}: No data provider found.",
                      path);
        return {};
    }

    auto start = high_resolution_clock::now();

    auto res_data = provider->load(relative_path);
    if (!res_data.valid()) {
        ARS_LOG_ERROR("Failed to load resources {}: Failed to load data.",
                      path);
        return {};
    }

    auto mid = high_resolution_clock::now();

    auto &ty = res_data.ty;
    auto loader_it = _res_loaders.find(ty);
    if (loader_it == _res_loaders.end()) {
        ARS_LOG_ERROR(
            "Failed to load resources {}: No loader registered for type {}.",
            path,
            ty);
        return {};
    }

    auto res = loader_it->second->load(std::move(res_data));

    auto stop = high_resolution_clock::now();

    auto total_duration = duration_cast<milliseconds>(stop - start);
    auto decode_duration = duration_cast<milliseconds>(mid - start);
    auto upload_duration = duration_cast<milliseconds>(stop - mid);

    ARS_LOG_INFO(
        "Load resource {} takes {}ms, fetch data takes {}ms, upload takes {}ms",
        path,
        total_duration.count(),
        decode_duration.count(),
        upload_duration.count());

    return res;
}

std::shared_ptr<IRes> IRes::get_sub_res(const std::string &name) const {
    auto it = _sub_res.find(name);
    if (it == _sub_res.end()) {
        ARS_LOG_ERROR("Subresource {} not found in resource {}", name, path());
        return {};
    }
    return it->second;
}

void IRes::set_sub_res(const std::string &name,
                       const std::shared_ptr<IRes> &res) {
    _sub_res[name] = res;
}

std::ostream &ResData::serialize(std::ostream &os) const {
    os.write(reinterpret_cast<const char *>(MAGIC_NUMBER), 4);
    nlohmann::json value = {
        {"ty", ty},
        {"meta", meta},
    };
    auto bs = nlohmann::json::to_bson(value);
    uint64_t meta_len = bs.size();
    uint64_t data_len = data.size();
    os.write(reinterpret_cast<const char *>(&meta_len), sizeof(meta_len));
    os.write(reinterpret_cast<const char *>(&data_len), sizeof(data_len));
    os.write(reinterpret_cast<const char *>(bs.data()),
             static_cast<std::streamsize>(bs.size()));
    os.write(reinterpret_cast<const char *>(data.data()),
             static_cast<std::streamsize>(data.size()));
    return os;
}

std::istream &ResData::deserialize(std::istream &is) {
    uint8_t magic[4]{};
    is.read(reinterpret_cast<char *>(magic), 4);
    if (std::memcmp(magic, MAGIC_NUMBER, 4) != 0) {
        ARS_LOG_ERROR("Invalid magic number for res data");
        reset();
        is.seekg(-4, std::ios::seekdir::cur);
        return is;
    }
    uint64_t meta_len{};
    uint64_t data_len{};
    is.read(reinterpret_cast<char *>(&meta_len), sizeof(meta_len));
    is.read(reinterpret_cast<char *>(&data_len), sizeof(data_len));
    std::vector<uint8_t> bs(meta_len);
    std::vector<uint8_t> rdata(data_len);

    is.read(reinterpret_cast<char *>(bs.data()),
            static_cast<std::streamsize>(meta_len));
    is.read(reinterpret_cast<char *>(rdata.data()),
            static_cast<std::streamsize>(data_len));

    auto value = nlohmann::json::from_bson(bs);
    ty = value.at("ty").get<std::string>();
    meta = value.at("meta");
    data = std::move(rdata);

    return is;
}

std::filesystem::path preferred_res_path(const std::filesystem::path &path) {
    auto preferred_ext = ".ares";
    if (path.extension() == preferred_ext) {
        return path;
    }
    return path.string() + preferred_ext;
}

void ResData::save(const std::filesystem::path &path) const {
    std::ofstream os(path, std::ios::binary);
    serialize(os);
    os.close();
}

void ResData::load(const std::filesystem::path &path) {
    std::ifstream is(path, std::ios::binary);
    deserialize(is);
    is.close();
}

bool ResData::valid() const {
    return !ty.empty();
}

void ResData::reset() {
    *this = ResData();
}

std::string IRes::path() const {
    return _path;
}

void IRes::set_path(const std::string &path) {
    _path = path;
}
} // namespace ars