#pragma once

#include "gl_types.h"
#include "gl_shader.h"
#include "gl_stock_shaders.h"

#include <cstdint>
#include <unordered_map>

namespace glengine {

// class Resource {
//   public:
//     ID id = NULL_ID;
//     std::string name = "";
//
//   protected:
//     Resource(ID id_, const std::string &name_)
//     : id(id_)
//     , name(name_) {}
// };


class ResourceManager {
  public:
    virtual ~ResourceManager();

    bool init();

    bool terminate();

    // /////// //
    // shaders //
    // /////// //

    /// create a new (uninitialized) shader
    Shader *create_shader();
    /// get shader by id
    Shader *get_shader(ID id);
    /// check if the shader with the given id exists
    bool has_shader(ID id) const;
    /// get stock shader
    Shader *get_stock_shader(StockShader type);

  protected:

    std::unordered_map<ID, Shader *> _shaders;
    std::unordered_map<StockShader, Shader *> _stock_shaders;

    ID _next_shader_id = 1;

    void create_stock_shaders();
};

} // namespace glengine
