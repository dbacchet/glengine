#pragma once

#include "gl_engine.h"
#include "grid.h"

template <typename CellT>
class GridRenderer {
public:
    GridRenderer(glengine::GLEngine *eng, Grid2D<CellT> *grid)
    : _eng(eng), _grid(grid) {}
    ~GridRenderer() = default;

    bool init() {
        create_grid();
        update();
        return false;
    }
    void update();
protected:

    glengine::GLEngine *_eng = nullptr;
    Grid2D<CellT> *_grid = nullptr;

    glengine::Object *_grid_obj = nullptr;
    std::vector<glengine::Object*> _grid_tiles;

    void create_grid();

    void create_tiles();
};

template <typename CellT>
void GridRenderer<CellT>::create_grid() {
    glengine::MeshData md;
    const int32_t major = 10;
    auto [originx, originy] = _grid->origin();
    auto [lenx, leny] = _grid->len();
    float step = _grid->cell_len();
    int32_t nlines_x = lenx / step;
    int32_t nlines_y = leny / step;
    glengine::Color col1 = {140, 140, 140, 255};
    // horizontal lines
    for (int32_t i = 0; i <= nlines_y; i++) {
        md.vertices.push_back({{0, i * step, 0}, col1});
        md.vertices.push_back({{float(lenx), i * step, 0}, col1});
    }
    // vertical lines
    for (int32_t i = 0; i <= nlines_x; i++) {
        md.vertices.push_back({{i * step, 0, 0}, col1});
        md.vertices.push_back({{i * step, float(leny), 0}, col1});
    }
    // create mesh
    glengine::Mesh *m = _eng->create_mesh();
    m->init(md.vertices, md.indices);
    // create renderable
    glengine::Renderable grid_renderable = { m, _eng->create_material<glengine::MaterialVertexColor>(SG_PRIMITIVETYPE_LINES, SG_INDEXTYPE_NONE)};
    // add renderables to the scene
    _grid_obj = _eng->create_object(grid_renderable); // renderable is _copied_ in the renderobject
    math::set_translation(_grid_obj->_transform, {float(originx), float(originy), 0.0f});
    // add a ui function to interact with the grid
    _eng->add_ui_function([&]() {
            ImGui::Begin("Grid Info");
            auto &gmat = _grid_obj->_transform;
            ImGui::DragFloat("grid x", &gmat.at(3,0), 1, -1000, 1000);
            ImGui::DragFloat("grid y", &gmat.at(3,1), 1, -1000, 1000);
            ImGui::DragFloat("grid z", &gmat.at(3,2), 1, -1000, 1000);
            ImGui::End();
        });
    // create the tiles and set them to non-visible
    create_tiles();
}

template <typename CellT>
void GridRenderer<CellT>::create_tiles() {
    float step = _grid->cell_len();
    glengine::MeshData md;
    // 0 1
    // *---*1 1
    // |\  |
    // | \ |
    // |  \|
    // *---*1 0
    // 0 0  
    md.vertices = {{{ 0.0f, step, 0.0f}, {255,155,155,255}},//, { 0, 0, 1}, {0,1}}, // this variant will draw to the top right quarter of the viewport
                   {{ 0.0f, 0.0f, 0.0f}, {255,155,155,255}},//, { 0, 0, 1}, {0,0}},
                   {{ step, 0.0f, 0.0f}, {255,155,155,255}},//, { 0, 0, 1}, {1,0}},
                   {{ 0.0f, step, 0.0f}, {255,155,155,255}},//, { 0, 0, 1}, {0,1}},
                   {{ step, 0.0f, 0.0f}, {255,155,155,255}},//, { 0, 0, 1}, {1,0}},
                   {{ step, step, 0.0f}, {255,155,155,255}},//, { 0, 0, 1}, {1,1}}, 
    };
    // md.vertices = {{{-1.0f, 1.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {0,1}},
    //                {{-1.0f,-1.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {0,0}},
    //                {{ 1.0f,-1.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {1,0}},
    //                {{-1.0f, 1.0f, 0.0f}, {255,255,255,255}, { 0, 0, 1}, {0,1}},
    //                {{ 1.0f,-1.0f,-0.0f}, {255,255,255,255}, { 0, 0,-1}, {1,0}},
    //                {{ 1.0f, 1.0f,-0.0f}, {255,255,255,255}, { 0, 0,-1}, {1,1}}, };
    // mesh
    glengine::Mesh *mesh = _eng->create_mesh(md.vertices, md.indices);

    // material
    auto *mtl_flat = _eng->create_material<glengine::MaterialFlat>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_NONE);
    mtl_flat->color = {255,255,255,255};
    // renderables
    glengine::Renderable renderable{mesh, mtl_flat};
    // objects
    auto [ntiles_x, ntiles_y] = _grid->size();
    for (int j=0; j<ntiles_y; j++) {
        for (int i=0; i<ntiles_x; i++) {
            auto *obj = _eng->create_object(renderable, _grid_obj);
            math::set_translation(obj->_transform, {i*step,j*step,0.0f});
            obj->set_visible(false);
            _grid_tiles.push_back(obj);
            // printf("%f %f %f\n",i*step, j*step, 0.0f);
        }
    }
}

template <typename CellT>
void GridRenderer<CellT>::update() {
    const auto &data = _grid->data();
    for (int i=0; i<data.size(); i++) {
        _grid_tiles[i]->set_visible(data[i]>0);
    }
}

