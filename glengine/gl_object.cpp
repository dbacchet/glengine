#include "gl_object.h"
#include "gl_camera.h"
#include "gl_mesh.h"
#include "gl_renderer.h"
#include "gl_shader.h"

#include "microprofile/microprofile.h"

#include <vector>
#include <set>

namespace glengine {

Object::Object(Object *parent, ID id)
: _parent(parent)
, _id(id) {
    if (_parent) {
        parent->add_child(this);
    }
}

Object::~Object() {
    if (_parent) {
        _parent->detach_child(this);
    }
    while (_children.size()>0) { // have to iterate this way because a regulare iterator gets invalidated when deleting
        delete *_children.begin();
    }
}

bool Object::init(Mesh *mesh, Shader *shader) {
    if (!mesh || !shader) {
        return false;
    }
    _meshes.push_back(mesh);
    _shader = shader;
    return true;
}

bool Object::init(std::vector<Mesh *> meshes, Shader *shader) {
    if (!shader) {
        return false;
    }
    _meshes = meshes;
    _shader = shader;
    return true;
}

bool Object::init(const std::vector<Renderable> &renderables) {
    _renderables = renderables;
    _shader = nullptr;
    return true;
}

bool Object::add_renderable(const Renderable *r, uint32_t num) {
    _renderables.insert(_renderables.end(), r, r+num);
    return true;
}

void Object::add_child(Object *ro) {
    if (!ro) {
        return;
    }
    // update previous parent
    if (ro->parent()) {
        ro->parent()->detach_child(ro);
    }
    ro->_parent = this;
    _children.insert(ro);
}

Object *Object::detach_child(Object *ro) {
    auto it = _children.find(ro);
    if (it != _children.end()) {
        Object *orphan = *it;
        orphan->_parent = nullptr;
        _children.erase(it);
        return orphan;
    }
    return nullptr;
}

bool Object::draw(Renderer &renderer, const Camera &cam, const math::Matrix4f &parent_tf) {
    MICROPROFILE_SCOPEI("renderobject","draw",MP_AUTO);
    math::Matrix4f curr_tf = parent_tf * _transform * _scale;
    if (_visible) {
        for (auto &go : _renderables) {
            MICROPROFILE_SCOPEI("renderobject","render_renderables",MP_AUTO);
            renderer.render_items.push_back({&cam, &go, curr_tf, _id});
        }
        if (_shader) {
            _shader->activate();
            _shader->set_uniform_id(_id);
            _shader->set_uniform_model(curr_tf);
            _shader->set_uniform_view(cam.inverse_transform());
            _shader->set_uniform_projection(cam.projection());
            _shader->set_uniform_light0_pos(math::Vector3f(100, 100, 100));
            for (auto m : _meshes) {
                MICROPROFILE_SCOPEI("renderobject","render_mesh",MP_AUTO);
                m->draw(*_shader);
            }
        }
        for (auto &c : _children) {
            c->draw(renderer, cam, curr_tf);
        }
    }
    return true;
}

Object &Object::set_transform(const math::Matrix4f &tf) {
    _transform = tf;
    return *this;
}
Object &Object::set_scale(const math::Vector3f &scl) {
    _scale(0, 0) = scl[0];
    _scale(1, 1) = scl[1];
    _scale(2, 2) = scl[2];
    return *this;
}
Object &Object::set_visible(bool flag) {
    _visible = flag;
    return *this;
}

} // namespace glengine
