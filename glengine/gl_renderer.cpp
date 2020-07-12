#include "gl_renderer.h"
#include "gl_camera.h"
#include "gl_mesh.h"
#include "gl_material.h"
#include "gl_renderable.h"

#include <vector>

namespace glengine {

bool Renderer::render() {
    for (auto &ri : render_items) {
        if (!ri.renderable || !ri.renderable->mesh || !ri.renderable->material || !ri.camera) {
            continue;
        }
        auto &mesh = *ri.renderable->mesh;
        auto &material = *ri.renderable->material;
        auto &camera = *ri.camera;
        // draw mesh
        glBindVertexArray(mesh.vao);
        material.apply(ri.id, ri.model_tf, camera.inverse_transform(), camera.projection());
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
