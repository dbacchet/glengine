#pragma once

#include "math/vmath.h"

#include <vector>

namespace glengine {

class Camera;
class Shader;
// class Material;
class Mesh;

struct RenderItem {
    Camera *camera = nullptr;
    // Material *material = nullptr;
    Mesh *mesh = nullptr;
    math::Matrix4f model_tf;
};

class Renderer {
public:

    bool render();

    std::vector<RenderItem> render_items;
};

}
