#include "gl_renderable.h"

#include "gl_mesh.h"
#include "gl_material.h"

namespace glengine {

void Renderable::update_bindings() {
    assert(mesh && "invalid mesh pointer");
    assert(material && "invalid material pointer");
    mesh->update_bindings(bind);
    material->update_bindings(bind);
}

void Renderable::apply_pipeline() {
    sg_apply_pipeline(material->pip);
}

void Renderable::apply_bindings() {
    sg_apply_bindings(bind);
}

void Renderable::apply_uniforms(const common_uniform_params_t &params) {
    material->apply_uniforms(params);
}

void Renderable::draw() {
    if (mesh->indices.size() > 0) {
        sg_draw(0, mesh->indices.size(), 1);
    } else {
        sg_draw(0, mesh->vertices.size(), 1);
    }
}

} // namespace glengine
