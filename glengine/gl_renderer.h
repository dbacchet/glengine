#pragma once

#include "gl_types.h"

#include "math/vmath.h"

#include <vector>

namespace glengine {

class Camera;
class Shader;
// class Material;
class Mesh;

struct RenderItem {
    const Camera *camera = nullptr;
    // Material *material = nullptr;
    const Mesh *mesh = nullptr;
    math::Matrix4f model_tf;
    ID id = NULL_ID;
};

class Renderer {
public:

    bool render();

    std::vector<RenderItem> render_items;
};

}
