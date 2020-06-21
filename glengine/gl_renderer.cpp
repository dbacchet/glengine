#include "gl_renderer.h"
#include "gl_camera.h"
#include "gl_mesh.h"
#include "gl_material.h"

#include <vector>

namespace glengine {

// struct RenderItem {
//     Camera *camera = nullptr;
//     // Material *material = nullptr;
//     Mesh *mesh = nullptr;
//     math::Matrix4f model_tf;
// };

bool Renderer::render() {
    for (auto &ri : render_items) {
        if (!ri.mesh || !ri.mesh->material || !ri.camera) {
            continue;
        }
        auto &mesh = *ri.mesh;
        auto &material = *ri.mesh->material;
        auto &camera = *ri.camera;
        // draw mesh
        glBindVertexArray(mesh.vao);
        material._shader->activate();
        material._shader->set_uniform_id(ri.id);
        material._shader->set_uniform_model(ri.model_tf);
        material._shader->set_uniform_view(camera.inverse_transform());
        material._shader->set_uniform_projection(camera.projection());
        material._shader->set_uniform_color(material.color);
        material._shader->set_uniform_light0_pos(math::Vector3f(100, 100, 100));
        material.bind_textures();
        if (mesh.indices.size() > 0) {
            glDrawElements(mesh.primitive, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        } else {
            glDrawArrays(mesh.primitive, 0, mesh.vertices.size());
        }
        glBindVertexArray(0);
    }
    return true;
}

} // namespace glengine
