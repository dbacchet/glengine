#pragma once

#include "gl_types.h"

#include "math/vmath.h"

#include <vector>

namespace glengine {

class Camera;
struct Renderable;

struct RenderItem {
    const Camera *camera = nullptr;
    const Renderable *renderable = nullptr;
    math::Matrix4f model_tf;
    ID id = NULL_ID;
};

class Renderer {
  public:
    bool render();

    std::vector<RenderItem> render_items;
};

} // namespace glengine
