#pragma once

#include <glm/glm.hpp>
#include <memory>

namespace ars::render {
class ITexture;

class IMaterial {
  public:
    enum Mode { Opaque, Blend };
    Mode alpha_mode() const;
    void set_alpha_mode(Mode alpha_mode);

    bool double_sided() const;
    void set_double_sided(bool double_sided);

    const glm::vec4 &base_color_factor() const;
    void set_base_color_factor(const glm::vec4 &base_color_factor);

    const std::shared_ptr<ITexture> &base_color_tex() const;
    void set_base_color_tex(const std::shared_ptr<ITexture> &base_color_tex);

    float metallic_factor() const;
    void set_metallic_factor(float metallic_factor);

    float roughness_factor() const;
    void set_roughness_factor(float roughness_factor);

    const std::shared_ptr<ITexture> &metallic_roughness_tex() const;
    void set_metallic_roughness_tex(
        const std::shared_ptr<ITexture> &metallic_roughness_tex);

    const std::shared_ptr<ITexture> &normal_tex() const;
    void set_normal_tex(const std::shared_ptr<ITexture> &normal_tex);

    float normal_scale() const;
    void set_normal_scale(float normal_scale);

    const std::shared_ptr<ITexture> &occlusion_tex() const;
    void set_occlusion_tex(const std::shared_ptr<ITexture> &occlusion_tex);

    float occlusion_strength() const;
    void set_occlusion_strength(float occlusion_strength);

    const std::shared_ptr<ITexture> &emission_tex() const;
    void set_emission_tex(const std::shared_ptr<ITexture> &emission_tex);

    const glm::vec3 &emission_factor() const;
    void set_emission_factor(const glm::vec3 &emission_factor);

    virtual ~IMaterial() = default;

  protected:
    Mode _alpha_mode = Opaque;
    bool _double_sided;
    glm::vec4 _base_color_factor;
    std::shared_ptr<ITexture> _base_color_tex{};
    float _metallic_factor;
    float _roughness_factor;
    std::shared_ptr<ITexture> _metallic_roughness_tex{};
    std::shared_ptr<ITexture> _normal_tex{};
    float _normal_scale;
    std::shared_ptr<ITexture> _occlusion_tex{};
    float _occlusion_strength;
    std::shared_ptr<ITexture> _emission_tex{};
    glm::vec3 _emission_factor{};
};
} // namespace ars::render
