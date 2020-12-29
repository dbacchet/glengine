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

} // namespace glengine
