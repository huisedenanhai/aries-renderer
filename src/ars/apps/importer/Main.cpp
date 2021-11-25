#include <ars/runtime/core/Log.h>
#include <ars/runtime/core/Res.h>
#include <ars/runtime/render/res/Texture.h>
#include <iostream>
#include <stb_image.h>
#include <tiny_gltf.h>

using namespace ars;

std::optional<ResData> import_texture(const std::filesystem::path &path) {
    int width, height, channels;
    auto path_str = path.string();
    // Only loads image as R8G8B8A8_SRGB for simplicity. Other formats may not
    // be supported on some devices.
    unsigned char *pixel_data =
        stbi_load(path_str.c_str(), &width, &height, &channels, 4);
    channels = 4;

    if (!pixel_data) {
        ARS_LOG_CRITICAL("Failed to load image {}", path.string());
        return std::nullopt;
    }

    render::TextureResMeta meta{};
    auto &info = meta.info;
    info.format = render::get_8_bit_texture_format(channels, true);
    info.width = width;
    info.height = height;

    DataSlice pixels{};
    pixels.offset = 0;
    pixels.size = width * height * channels;

    meta.layers.resize(1);
    meta.layers[0].mipmaps.push_back(pixels);

    ResData res{};
    res.ty = render::RES_TYPE_NAME_TEXTURE;
    res.meta = meta;
    res.data.resize(pixels.size);
    std::memcpy(res.data.data(), pixel_data, pixels.size);

    return res;
}

void gltf_warn(const std::filesystem::path &path, const std::string &info) {
    ARS_LOG_WARN("Loading {}: {}", path.string(), info);
}

void import_gltf_mesh(const std::filesystem::path &path,
                      const tinygltf::Model &gltf) {
    auto target_dir = ".ars" / path.parent_path();
    std::filesystem::create_directories(target_dir);
}

void import_gltf(const std::filesystem::path &path) {
    tinygltf::TinyGLTF loader;
    tinygltf::Model gltf;
    std::string err;
    std::string warn;

    using namespace std::chrono;
    auto start = high_resolution_clock::now();

    auto extension = path.extension();
    auto model_path_str = path.string();

    loader.SetLoadExternalImages(false);

    auto gltf_error = [&](const std::string &info) {
        ARS_LOG_ERROR("Failed to load {}: {}", model_path_str, info);
    };

    bool ret;
    if (extension == ".gltf") {
        ret = loader.LoadASCIIFromFile(&gltf, &err, &warn, model_path_str);
    } else if (extension == ".glb") {
        ret = loader.LoadBinaryFromFile(&gltf, &err, &warn, model_path_str);
    } else {
        std::stringstream ss;
        ss << "Invalid file extension for GLTF: " << extension
           << ", should be .gltf or .glb";
        gltf_error(ss.str());
        return;
    }

    if (!warn.empty()) {
        gltf_warn(path, warn);
    }

    if (!err.empty()) {
        gltf_error(err);
        return;
    }

    if (!ret) {
        ARS_LOG_ERROR("Failed to parse {}", path.string());
        return;
    }

    auto end = high_resolution_clock::now();

    ARS_LOG_INFO("Open gltf file {} takes {}ms",
                 path.string(),
                 duration_cast<milliseconds>(end - start).count());
    import_gltf_mesh(path, gltf);
}

int main() {
    ARS_LOG_INFO("Importing {}", "test.jpg");
    auto res = import_texture("test.jpg");
    if (res.has_value()) {
        res->save(preferred_res_path("test.jpg"));
    }
    auto gltf_path = "FlightHelmetWithLight/FlightHelmetWithLight.gltf";
    ARS_LOG_INFO("Importing {}", gltf_path);
    import_gltf(gltf_path);
}