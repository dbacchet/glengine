#include "annotations.h"

namespace {

math::Vector3d lle_to_vec3(const Point3d &p) {
    return {p.y(), p.x(), p.z()};
}
} // namespace

bool Annotations::init_from_file(const char *filename, const math::Vector3d &origin) {
    // load the map from the binary protobuf
    std::fstream input(filename, std::ios::in | std::ios::binary);
    if (!_annotations.ParseFromIstream(&input)) {
        return false;
    }
    printf("number of paths: %d\n", _annotations.paths_size());

    // WGS84 converter
    WGS84Converter conv;
    conv.set_origin(origin);

    // lines
    std::vector<glengine::Vertex> lines_points;
    lines_points.reserve(10000);
    for (const auto &it : _annotations.paths()) {
        // printf("%d\n", it.first);
        const auto &left_pline = it.second.left_boundary().line();
        for (int i = 1; i < left_pline.waypoints_size(); i++) {
            const auto &pt0 = left_pline.waypoints(i - 1);
            const auto &pt1 = left_pline.waypoints(i);
            lines_points.push_back({conv.wgs84_to_cart(lle_to_vec3(pt0))});
            lines_points.push_back({conv.wgs84_to_cart(lle_to_vec3(pt1))});
        }
        const auto &right_pline = it.second.right_boundary().line();
        for (int i = 1; i < right_pline.waypoints_size(); i++) {
            const auto &pt0 = right_pline.waypoints(i - 1);
            const auto &pt1 = right_pline.waypoints(i);
            lines_points.push_back({conv.wgs84_to_cart(lle_to_vec3(pt0))});
            lines_points.push_back({conv.wgs84_to_cart(lle_to_vec3(pt1))});
        }
        if (left_pline.waypoints_size() > 0) {
            // connect the initial points
            lines_points.push_back({conv.wgs84_to_cart(lle_to_vec3(left_pline.waypoints(0)))});
            lines_points.push_back({conv.wgs84_to_cart(lle_to_vec3(right_pline.waypoints(0)))});
            // connect the final points
            lines_points.push_back(
                {conv.wgs84_to_cart(lle_to_vec3(left_pline.waypoints(left_pline.waypoints_size() - 1)))});
            lines_points.push_back(
                {conv.wgs84_to_cart(lle_to_vec3(right_pline.waypoints(right_pline.waypoints_size() - 1)))});
        }
    }
    glengine::Mesh *annotations_mesh = _eng.resource_manager().create_mesh("lines");
    annotations_mesh->init(lines_points, GL_LINES);
    glengine::Renderable annotations_renderable = {annotations_mesh, lanes_material};
    auto &annotations_ro = *_eng.create_renderobject(annotations_renderable, root);

    // tags
    for (const auto &it : _annotations.paths()) {
        const auto &left_pline = it.second.left_boundary().line();
        const auto &right_pline = it.second.right_boundary().line();
        // extract tags
        for (int i = 0; i < it.second.tags_size(); i++) {
            const auto &tag = it.second.tags(i);
            const auto &sp = tag.start();
            const auto &ep = tag.end();
            std::cout << tag.tag_type() << " " << sp.x() << " " << sp.y() << " " << sp.z() << " " << ep.x() << " "
                      << ep.y() << " " << ep.z() << std::endl;
            if (tag.tag_type() == STOPLINE) {
                std::vector<math::Vector3f> stopline_points;
                stopline_points.push_back(
                    conv.wgs84_to_cart(lle_to_vec3(left_pline.waypoints(left_pline.waypoints_size() - 1))));
                stopline_points.push_back(
                    conv.wgs84_to_cart(lle_to_vec3(right_pline.waypoints(right_pline.waypoints_size() - 1))));
                glengine::Mesh *stopline_mesh = _eng.resource_manager().create_polyline_mesh(
                    "stopline", stopline_points.data(), stopline_points.size(), 0.4f);
                glengine::Renderable stopline_renderable = {stopline_mesh, stopline_material};
                auto stopline_ro = _eng.create_renderobject(stopline_renderable, root);
                // stopline marker
                glengine::Renderable stopline_renderable_marker = {_eng.resource_manager().create_sphere_mesh("stopline_marker",0.5f),
                                                                   stopline_material};
                auto stopline_ro_marker = _eng.create_renderobject(stopline_renderable_marker, root);
                stopline_ro_marker->set_transform(math::create_translation<float>({conv.wgs84_to_cart(lle_to_vec3(sp))}));
            }
            if (tag.tag_type() == PEDESTRIAN_CROSSING) {
                std::vector<math::Vector3f> points;
                // auto p1 = conv.wgs84_to_cart(lle_to_vec3(left_pline.waypoints(left_pline.waypoints_size() - 1)));
                // auto p2 = conv.wgs84_to_cart(lle_to_vec3(right_pline.waypoints(right_pline.waypoints_size() - 1)));
                // points.push_back( conv.wgs84_to_cart(lle_to_vec3(left_pline.waypoints(left_pline.waypoints_size() -
                // 1)))); points.push_back(
                // conv.wgs84_to_cart(lle_to_vec3(right_pline.waypoints(right_pline.waypoints_size() - 1))));
                points.push_back(conv.wgs84_to_cart(lle_to_vec3(tag.start())));
                points.push_back(conv.wgs84_to_cart(lle_to_vec3(tag.end())));
                glengine::Mesh *crosswalk_mesh =
                    _eng.resource_manager().create_polyline_mesh("crosswalk", points.data(), points.size(), 0.4f);
                glengine::Renderable crosswalk_renderable = {crosswalk_mesh, crosswalk_material};
                auto crosswalk_ro = _eng.create_renderobject(crosswalk_renderable, root);
                // glengine::Renderable stopline_renderable_marker = {_eng.resource_manager().create_sphere_mesh(),
                //                                                    stopline_material};
                // stopline_ro->add_renderable(&stopline_renderable_marker,1);
                // stopline_ro->set_transform(math::create_translation<float>({conv.wgs84_to_cart(lle_to_vec3(sp))}));
            }
        }
    }
    return true;
}
