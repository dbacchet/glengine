#pragma once

#include "gl_types.h"

#include "gl_shader.h"
#include "gl_texture.h"

#include <string>

namespace glengine {

class Shader;
class Texture;

struct Material {
    /// "standard" textures, modeled after the GLTF specs for PBR Materials:
    /// [here](https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/README.md#metallic-roughness-material)
    enum class TextureType {
        BaseColor = 0, ///< the base color texture (i.e. albedo, diffuse, etc)
        ORM,           ///< Occlusion, Roughness, Metallic (in R,G,B channels respectively)
        Normal,        ///< normal map in tangent space
        Emissive,      ///< color and intensity of the light emitted by the material
        TextureTypeNum
    };
    static constexpr char const *uniform_names[] = {
        "tex_basecolor", ///< standard uniform for the base color
        "tex_ORM",       ///< standard uniform for occlusion/roughness/metallic
        "tex_normal",    ///< standard uniform for the tangent space normals
        "tex_emissive"   ///< standard uniform for the emissive color
    };

    Material(const std::string &name_) 
    : name(name_) {}

    bool init(Shader *shader);

    std::string name = "";
    Shader *_shader = nullptr;
    Texture *_textures[(uint8_t)TextureType::TextureTypeNum] = {nullptr};

    glengine::Color color = {200, 100, 100, 255};
    math::Vector4f base_color_factor = {1.0f, 1.0f, 1.0f, 1.0f};
    math::Vector3f emissive_factor = {1.0f, 1.0f, 1.0f};
    float metallic_factor = 0.0f;
    float roughness_factor = 0.0f;
    float normal_scale = 1.0f;

    void bind_textures() {
        if (!_shader) {
            return;
        }
        for (uint16_t i = 0; i < (uint16_t)TextureType::TextureTypeNum; i++) {
            const auto tex = _textures[i];
            const auto uniform_name = uniform_names[i];
            // if the texture exists, has been created and is referenced in the shader, then bind it
            if (tex && tex->texture_id != NULL_TEXTURE_ID && _shader->has_uniform(uniform_name)) {
                glActiveTexture(GL_TEXTURE0 + i);
                glBindTexture(GL_TEXTURE_2D, tex->texture_id);
                _shader->set_sampler(uniform_name, i);
            }
        }
    }
};

} // namespace glengine
