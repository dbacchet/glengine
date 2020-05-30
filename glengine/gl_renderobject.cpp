#include "gl_renderobject.h"
#include "gl_camera.h"
#include "gl_mesh.h"
#include "gl_shader.h"

#include <algorithm>

namespace glengine {

RenderObject::RenderObject(ID id)
: _id(id) {}

RenderObject::~RenderObject() {
    if (_parent) {
        _parent->detach_child_by_id(_id);
    }
    for (auto c: _children) {
        delete c;
    }
    printf("deleted renderobject %u\n", (uint32_t)_id);
}

bool RenderObject::init(Mesh *mesh, Shader *shader) {
    if (!mesh || !shader) {
        return false;
    }
    _meshes.push_back(mesh);
    _shader = shader;
    return true;
}

bool RenderObject::init(std::vector<Mesh *> meshes, Shader *shader) {
    if (!shader) {
        return false;
    }
    _meshes = meshes;
    _shader = shader;
    return true;
}

void RenderObject::add_child(RenderObject *ro) {
    if (!ro) {
        return;
    }
    ro->_parent = this;
    _children.push_back(ro);
}

RenderObject *RenderObject::detach_child_by_id(ID id) {
    auto it = std::find_if(_children.begin(), _children.end(), [id](const RenderObject* ro){ return ro->_id==id; });
    if (it != _children.end()) {
        return *it;
    }
    return nullptr;
}


bool RenderObject::draw(const Camera &cam) {
    return draw(cam, math::matrix4_identity<float>());
}

bool RenderObject::draw(const Camera &cam, const math::Matrix4f &parent_tf) {
    math::Matrix4f curr_tf = parent_tf * _transform * _scale;
    if (_visible) {
        if (_shader) {
            _shader->activate();
            _shader->set_uniform_id(_id);
            _shader->set_uniform_model(curr_tf);
            _shader->set_uniform_view(cam.inverse_transform());
            _shader->set_uniform_projection(cam.projection());
            _shader->set_uniform_color(_color);
            _shader->set_uniform_light0_pos(math::Vector3f(100, 100, 100));
            for (auto m : _meshes) {
                m->draw(*_shader);
            }
        }
    }
    for (auto &c:_children) {
        c->draw(cam, curr_tf);
    }
    return true;
}

RenderObject &RenderObject::set_transform(const math::Matrix4f &tf) {
    _transform = tf;
    return *this;
}
RenderObject &RenderObject::set_scale(const math::Vector3f &scl) {
    _scale(0, 0) = scl[0];
    _scale(1, 1) = scl[1];
    _scale(2, 2) = scl[2];
    return *this;
}
RenderObject &RenderObject::set_color(const glengine::Color &color) {
    _color = color;
    return *this;
}
RenderObject &RenderObject::set_visible(bool flag) {
    _visible = flag;
    return *this;
}

} // namespace glengine
