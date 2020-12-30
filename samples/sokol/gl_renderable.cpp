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
void Renderable::apply_uniforms() {
    material->apply_uniforms();
}

} // namespace glengine
