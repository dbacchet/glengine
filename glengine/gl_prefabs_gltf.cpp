#include "math/vmath.h"
#include "gl_prefabs.h"

#include "tinygltf/tiny_gltf.h"

#include <vector>
#include <iostream>

namespace {

std::vector<glengine::MeshData> model_meshes;

bool bindMesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh) {
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
            indexAccessor.type != TINYGLTF_TYPE_SCALAR || indexAccessor.componentType != TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
            printf("SKIP: vertex/index data format not supported yet\n");
            continue;
        }
        // position
        printf("        has POSITION (%lu)\n", pos_accessor.count);
        const tinygltf::BufferView &pos_bufferView = model.bufferViews[pos_accessor.bufferView];
        const tinygltf::Buffer &pos_buffer = model.buffers[pos_bufferView.buffer];
        math::Vector3f *positions = (math::Vector3f *)(pos_buffer.data.data() + pos_bufferView.byteOffset + pos_accessor.byteOffset);
        // for (int ii = 0; ii < pos_accessor.count; ii++) {
        //     auto &p = positions[ii];
        //     printf("%4d pos: (%f %f %f)\n", ii, p.x, p.y, p.z);
        //     if (ii > 3)
        //         break;
        // }
        // normal
        printf("        has NORMAL (%lu)\n", norm_accessor.count);
        const tinygltf::BufferView &norm_bufferView = model.bufferViews[norm_accessor.bufferView];
        const tinygltf::Buffer &norm_buffer = model.buffers[norm_bufferView.buffer];
        math::Vector3f *normals = (math::Vector3f *)(norm_buffer.data.data() + norm_bufferView.byteOffset + norm_accessor.byteOffset);
        // for (int ii = 0; ii < norm_accessor.count; ii++) {
        //     auto &p = normals[ii];
        //     printf("%4d norm: (%f %f %f)\n", ii, p.x, p.y, p.z);
        //     if (ii > 3)
        //         break;
        // }
        // texcoord
        printf("        has TEXCOORD_0 (%lu)\n", tc0_accessor.count);
        const tinygltf::BufferView &tc0_bufferView = model.bufferViews[tc0_accessor.bufferView];
        const tinygltf::Buffer &tc0_buffer = model.buffers[tc0_bufferView.buffer];
        math::Vector2f *texcoords = (math::Vector2f *)(tc0_buffer.data.data() + tc0_bufferView.byteOffset + tc0_accessor.byteOffset);
        // for (int ii = 0; ii < tc0_accessor.count; ii++) {
        //     auto &p = texcoords[ii];
        //     printf("%4d norm: (%f %f)\n", ii, p.x, p.y);
        //     if (ii > 3)
        //         break;
        // }
        // indices
        const tinygltf::BufferView &indexbufferView = model.bufferViews[indexAccessor.bufferView];
        const tinygltf::Buffer &indexbuffer = model.buffers[indexbufferView.buffer];
        uint32_t *indices = (uint32_t *)(indexbuffer.data.data() + indexbufferView.byteOffset + indexAccessor.byteOffset);
        // for (int ii = 0; ii < indexAccessor.count; ii++) {
        //     printf("%u ",indices[ii]);
        //     if (ii > 30)
        //         break;
        // }
        // create vertices
        glengine::MeshData md;
        for (int vi=0; vi<pos_accessor.count; vi++) {
            md.vertices.push_back({positions[vi],{150,150,150,255},normals[vi],{texcoords[vi].s,texcoords[vi].t}});
        }
        for (int ii=0; ii<indexAccessor.count; ii++) {
            md.indices.push_back(indices[ii]);
        }
        model_meshes.push_back(md);
    }

    return true;
}

void bindModelNodes(tinygltf::Model &model, tinygltf::Node &node) {
    printf("node '%s'\n", node.name.c_str());
    if ((node.mesh >= 0) && (node.mesh < model.meshes.size())) {
        bindMesh(model, model.meshes[node.mesh]);
        return; // return after binding the first mesh
    }

    for (size_t i = 0; i < node.children.size(); i++) {
        assert((node.children[i] >= 0) && (node.children[i] < model.nodes.size()));
        bindModelNodes(model, model.nodes[node.children[i]]);
    }
}

} // namespace

namespace glengine {

MeshData create_from_gltf(const char *filename) {
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
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        assert((scene.nodes[i] >= 0) && (scene.nodes[i] < model.nodes.size()));
        auto &node = model.nodes[scene.nodes[i]];
        // printf("node '%s'\n", node.name.c_str());
        bindModelNodes(model, model.nodes[scene.nodes[i]]);
    }
    printf("created %lu meshdata structs\n",model_meshes.size());
    return model_meshes[0];
}

} // namespace glengine
