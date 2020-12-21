#include "pcd_tiles.h"

#include "math/vmath.h"
#include "pcd_loader.h"

#include <string>
#include <sstream>
#include <unordered_set>
#include <filesystem>

#include <iostream>

namespace fs = std::filesystem;

void PCDTiles::update_tiles(const math::Vector2i &center, int _radius) {
    if (center == tile_center && _radius==radius) {
        return;
    }
    tile_center = center;
    radius = std::max(_radius, 0);

    // prepare the list of visible tiles
    std::unordered_set<std::string> tiles;
    for (int32_t tile_i = tile_center.x - radius; tile_i <= tile_center.x + radius; tile_i++) {
        for (int32_t tile_j = tile_center.y - radius; tile_j <= tile_center.y + radius; tile_j++) {
            std::stringstream tile_path;
            tile_path << tile_dir << "/" << tile_i << "_" << tile_j << ".pcd";
            tiles.insert(tile_path.str());
        }
    }
    // remove tiles that are not in the radius
    std::unordered_set<std::string> current_tiles;
    for (const auto &it : _tile_objects) {
        current_tiles.insert(it.first);
    }
    for (const auto &tname : current_tiles) {
        if (tiles.count(tname) == 0) {
            printf("remove tile: %s\n", tname.c_str());
            delete _tile_objects[tname];
            _tile_objects.erase(tname);
        }
    }
    // load missing tiles
    for (const auto &tname : tiles) {
        if (current_tiles.count(tname) == 0 && fs::exists(tname)) {
            printf("load tile: %s\n", tname.c_str());
            // this is a good candidate for async execution, but I need to figure out how to make the OpenGL context available in every thread
            std::vector<glengine::Vertex> points;
            points.reserve(10000); // to reduce mem allocations
            if (load_pcd(tname.c_str(), points)) {
                auto &rm = _eng.resource_manager();
                auto points_mesh = rm.create_mesh(tname.c_str());
                points_mesh->init(points, GL_POINTS);
                glengine::Renderable points_renderable = {points_mesh, points_material};
                auto points_ro = _eng.create_renderobject(points_renderable, root);
                _tile_objects[tname] = points_ro;
            }

        }
    }
}

