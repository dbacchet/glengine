#pragma once

#include "gl_types.h"

#include "gl_shader.h"
#include "gl_texture.h"

#include <string>

namespace glengine {

class Shader;
class Texture;

struct Material: public Resource {
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

    Material(ID id, const std::string &name, Shader *shader) 
    : Resource(id, name), _shader(shader){}

    bool init(Shader *shader) {
        _shader = shader;
        return _shader;
    }

    Shader *_shader = nullptr;
    Texture *_textures[(uint8_t)TextureType::TextureTypeNum] = {nullptr};

    math::Vector4f base_color_factor = {1.0f, 1.0f, 1.0f, 1.0f};
    math::Vector3f emissive_factor = {1.0f, 1.0f, 1.0f};
    float metallic_factor = 0.0f;
    float roughness_factor = 0.0f;
    float normal_scale = 1.0f;

    void apply(ID object_id, const math::Matrix4f &model_tf, const math::Matrix4f &view_tf, const math::Matrix4f &proj_tf, const math::Vector3f &light_pos={100.0f, 100.0f, 100.0f}) {
        _shader->activate();
        // standard attributes (common to all materials)
        _shader->set_uniform_id(object_id);
        _shader->set_uniform_model(model_tf);
        _shader->set_uniform_view(view_tf);
        _shader->set_uniform_projection(proj_tf);
        _shader->set_uniform_light0_pos(light_pos);
        // shader specific attributes
        _shader->set_vec4("u_color", base_color_factor);
        bind_textures();
    }

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
