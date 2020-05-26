#include "math/vmath.h"
#include "gl_prefabs.h"

#include "tinygltf/tiny_gltf.h"

#include <vector>
#include <iostream>

namespace {

std::vector<glengine::MeshData> model_meshes; /// \todo get rid of this globa var

math::Matrix4f extract_transform(const tinygltf::Node &node) {
    math::Matrix4f tf = math::matrix4_identity<float>();
    const auto &m = node.matrix;
    const auto &t = node.translation;
    const auto &r = node.rotation;
    const auto &s = node.scale;
    // the node can _either_ have a tf matrix or a set of T/R/S data
    if (m.size() > 0) {
        for (int i = 0; i < 16; i++) {
            tf.data[i] = m[i];
        }
    } else if (t.size() >= 3 || r.size() >= 4 || s.size() >= 3) {
        math::Vector3f translation = {0, 0, 0};
        math::Vector3f scale = {1, 1, 1};
        math::Quatf rotation = {1, 0, 0, 0};
        if (t.size() >= 3) {
            translation = {float(t[0]), float(t[1]), float(t[2])};
        }
        if (s.size() >= 3) {
            scale = {float(s[0]), float(s[1]), float(s[2])};
        }
        if (r.size() >= 4) {
            rotation = {float(r[3]), float(r[0]), float(r[1]), float(r[2])};
        }
        tf = math::create_translation(translation) * math::create_transformation({0.0f, 0.0f, 0.0f}, rotation) *
             math::create_scaling(scale);
    }
    return tf;
}

bool bindMesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh, const math::Matrix4f &tf) {
    printf("[bind] mesh: %s\n", mesh.name.c_str());

    for (size_t pi = 0; pi < mesh.primitives.size(); ++pi) {
        tinygltf::Primitive primitive = mesh.primitives[pi];
        printf("  [bind] primitive %d\n", primitive.mode);
        if (primitive.mode != TINYGLTF_MODE_TRIANGLES || primitive.attributes.count("POSITION") == 0 ||
            primitive.attributes.count("NORMAL") == 0 || primitive.attributes.count("TEXCOORD_0") == 0) {
            printf("         SKIP (only triangles with pos, normal and texcoord0 are supported for now)!\n");
            continue;
        }
        // accessors
        const tinygltf::Accessor &pos_accessor = model.accessors[primitive.attributes["POSITION"]];
        const tinygltf::Accessor &norm_accessor = model.accessors[primitive.attributes["NORMAL"]];
        const tinygltf::Accessor &tc0_accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
        const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
        if (pos_accessor.type != TINYGLTF_TYPE_VEC3 || pos_accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
            norm_accessor.type != TINYGLTF_TYPE_VEC3 || norm_accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
            tc0_accessor.type != TINYGLTF_TYPE_VEC2 || tc0_accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT ||
            indexAccessor.type != TINYGLTF_TYPE_SCALAR ||
            indexAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            printf("SKIP: vertex/index data format not supported yet\n");
            continue;
        }
        // position
        const tinygltf::BufferView &pos_bufferView = model.bufferViews[pos_accessor.bufferView];
        const tinygltf::Buffer &pos_buffer = model.buffers[pos_bufferView.buffer];
        math::Vector3f *positions =
            (math::Vector3f *)(pos_buffer.data.data() + pos_bufferView.byteOffset + pos_accessor.byteOffset);
        // normal
        const tinygltf::BufferView &norm_bufferView = model.bufferViews[norm_accessor.bufferView];
        const tinygltf::Buffer &norm_buffer = model.buffers[norm_bufferView.buffer];
        math::Vector3f *normals =
            (math::Vector3f *)(norm_buffer.data.data() + norm_bufferView.byteOffset + norm_accessor.byteOffset);
        // texcoord
        const tinygltf::BufferView &tc0_bufferView = model.bufferViews[tc0_accessor.bufferView];
        const tinygltf::Buffer &tc0_buffer = model.buffers[tc0_bufferView.buffer];
        math::Vector2f *texcoords =
            (math::Vector2f *)(tc0_buffer.data.data() + tc0_bufferView.byteOffset + tc0_accessor.byteOffset);
        // indices
        const tinygltf::BufferView &indexbufferView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer &indexbuffer = model.buffers[indexbufferView.buffer];
        uint32_t *indices =
            (uint32_t *)(indexbuffer.data.data() + indexbufferView.byteOffset + indexAccessor.byteOffset);
        // create vertices
        glengine::MeshData md;
        auto rot = tf;
        math::set_translation(rot, {0, 0, 0});
        for (int vi = 0; vi < pos_accessor.count; vi++) {
            md.vertices.push_back(
                {tf * positions[vi], {150, 150, 150, 255}, rot * normals[vi], {texcoords[vi].s, texcoords[vi].t}});
        }
        for (int ii = 0; ii < indexAccessor.count; ii++) {
            md.indices.push_back(indices[ii]);
        }
        model_meshes.push_back(md);
    }

    return true;
}

void bindModelNodes(tinygltf::Model &model, tinygltf::Node &node, const math::Matrix4f &parent_tf) {
    printf("node '%s'\n", node.name.c_str());
    math::Matrix4f tf = extract_transform(node);
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
        bindMesh(model, model.meshes[node.mesh], parent_tf * tf);
        return; // return after binding the first mesh
    }

    for (size_t i = 0; i < node.children.size(); i++) {
        assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
        bindModelNodes(model, model.nodes[node.children[i]], parent_tf * tf);
    }
}

} // namespace

namespace glengine {

std::vector<MeshData> create_from_gltf(const char *filename) {
    MeshData md;
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    // bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, argv[1]); // for binary glTF(.glb)

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }

    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }

    if (!ret) {
        printf("Failed to parse glTF\n");
    }

    printf("the model has %lu buffers\n", model.buffers.size());
    const tinygltf::Scene &scene = model.scenes[model.defaultScene];
    printf("the scene has %lu nodes\n", scene.nodes.size());
    math::Matrix4f root_tf = math::matrix4_identity<float>();
    root_tf = math::create_transformation({0,0,0},math::quat_from_euler_321<float>(M_PI_2,0,0)); // because by default gltf are y-up
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
        auto &node = model.nodes[scene.nodes[i]];
        bindModelNodes(model, model.nodes[scene.nodes[i]], root_tf);
    }
    printf("created %lu meshdata structs\n", model_meshes.size());
    // // flatten meshes for now, since we are not reading the material properties for each one
    // for (const auto &m : model_meshes) {
    //     uint32_t offs = md.vertices.size();
    //     md.vertices.insert(md.vertices.end(), m.vertices.begin(), m.vertices.end());
    //     for (uint32_t idx : m.indices) {
    //         md.indices.push_back(idx + offs);
    //     }
    // }
    // return md;
    return model_meshes;
}

} // namespace glengine
