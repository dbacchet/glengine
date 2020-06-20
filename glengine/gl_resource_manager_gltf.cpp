#include "gl_resource_manager.h"
#include "gl_prefabs.h"
#include "gl_logger.h"

#include "math/vmath.h"
#include "stb/stb_image.h"
#include "tinygltf/tiny_gltf.h"

#include <set>

using namespace glengine;

namespace {

std::string get_file_extension(const std::string& filename) {
    if(filename.find_last_of(".") != std::string::npos)
        return filename.substr(filename.find_last_of(".")+1);
    return "";
}

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

class GltfLoader {
  public:
    GltfLoader(const std::string &filename, ResourceManager &rm)
    : _filename(filename), _rm(rm) {}

    bool load_mesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh, const math::Matrix4f &tf) {
        for (size_t pi = 0; pi < mesh.primitives.size(); ++pi) {
            tinygltf::Primitive primitive = mesh.primitives[pi];
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES || primitive.attributes.count("POSITION") == 0) {
                printf("         SKIP (only triangles are supported for now)!\n");
                continue;
            }
            // position
            const tinygltf::Accessor &pos_accessor = model.accessors[primitive.attributes["POSITION"]];
            if (pos_accessor.type != TINYGLTF_TYPE_VEC3 ||
                pos_accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT) {
                printf("SKIP: position vertex/index data format not supported yet\n");
                continue;
            }
            const tinygltf::BufferView &pos_bufferView = model.bufferViews[pos_accessor.bufferView];
            const tinygltf::Buffer &pos_buffer = model.buffers[pos_bufferView.buffer];
            math::Vector3f *positions =
                (math::Vector3f *)(pos_buffer.data.data() + pos_bufferView.byteOffset + pos_accessor.byteOffset);
            // normal
            math::Vector3f *normals = nullptr;
            if (primitive.attributes.count("NORMAL") != 0) {
                const tinygltf::Accessor &norm_accessor = model.accessors[primitive.attributes["NORMAL"]];
                if (norm_accessor.type == TINYGLTF_TYPE_VEC3 ||
                    norm_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    const tinygltf::BufferView &norm_bufferView = model.bufferViews[norm_accessor.bufferView];
                    const tinygltf::Buffer &norm_buffer = model.buffers[norm_bufferView.buffer];
                    normals = (math::Vector3f *)(norm_buffer.data.data() + norm_bufferView.byteOffset +
                                                 norm_accessor.byteOffset);
                }
            }
            // texcoord
            math::Vector2f *texcoords = nullptr;
            if (primitive.attributes.count("TEXCOORD_0") != 0) {
                const tinygltf::Accessor &tc0_accessor = model.accessors[primitive.attributes["TEXCOORD_0"]];
                if (tc0_accessor.type == TINYGLTF_TYPE_VEC2 ||
                    tc0_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    const tinygltf::BufferView &tc0_bufferView = model.bufferViews[tc0_accessor.bufferView];
                    const tinygltf::Buffer &tc0_buffer = model.buffers[tc0_bufferView.buffer];
                    texcoords = (math::Vector2f *)(tc0_buffer.data.data() + tc0_bufferView.byteOffset +
                                                   tc0_accessor.byteOffset);
                }
            }
            // indices
                uint32_t *indices_int = nullptr;
                uint16_t *indices_short = nullptr;
            const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
            if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT ||
                 indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                const tinygltf::BufferView &indexbufferView = model.bufferViews[indexAccessor.bufferView];
                const tinygltf::Buffer &indexbuffer = model.buffers[indexbufferView.buffer];
                indices_int =
                    (uint32_t *)(indexbuffer.data.data() + indexbufferView.byteOffset + indexAccessor.byteOffset);
                indices_short =
                    (uint16_t *)(indexbuffer.data.data() + indexbufferView.byteOffset + indexAccessor.byteOffset);
            } else {
                printf("SKIP indices: index data format not supported yet\n");
            }
            // create vertices
            if (pos_accessor.count <3) {
                printf("empty mesh\n");
                continue;
            }
            Mesh *glmesh = _rm.create_mesh((_filename + std::string("_") + mesh.name).c_str());
            glengine::MeshData md;
            auto rot = tf;
            math::set_translation(rot, {0, 0, 0});
            for (uint32_t vi = 0; vi < pos_accessor.count; vi++) {
                auto pos = tf * positions[vi];
                auto norm = normals ? rot * normals[vi] : math::Vector3f{0.0f,0.0f,0.0f};
                auto tc0 = texcoords ? math::Vector2f{texcoords[vi].s, texcoords[vi].t} : math::Vector2f{0.0f,0.0f};
                md.vertices.push_back(
                    {pos, {150, 150, 150, 255}, norm, tc0});
            }
            if (indexAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                for (uint32_t ii = 0; ii < indexAccessor.count; ii++) {
                    md.indices.push_back(indices_int[ii]);
                }
            } else {
                for (uint32_t ii = 0; ii < indexAccessor.count; ii++) {
                    md.indices.push_back(indices_short[ii]);
                }
            }
            // material
            glmesh->init(md.vertices, md.indices, GL_TRIANGLES);
            if (primitive.material >= 0) {
                const tinygltf::Material &material = model.materials[primitive.material];
                if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                    glmesh->textures.diffuse = _tx_map[material.pbrMetallicRoughness.baseColorTexture.index];
                }
            }
            _meshes.push_back(glmesh);
        }
        return true;
    }

    void load_node(const tinygltf::Model &model, const tinygltf::Node &node, const math::Matrix4f &parent_tf) {
        math::Matrix4f tf = extract_transform(node);
        if ((node.mesh >= 0) && (node.mesh < int(model.meshes.size()))) {
            load_mesh(model, model.meshes[node.mesh], parent_tf * tf);
        }

        for (size_t i = 0; i < node.children.size(); i++) {
            assert((node.children[i] >= 0) && (node.children[i] < int(model.nodes.size())));
            load_node(model, model.nodes[node.children[i]], parent_tf * tf);
        }
    }

    bool load_textures(const tinygltf::Model &model) {
        for (uint32_t i = 0; i < model.images.size(); i++) {
            const tinygltf::Image &img = model.images[i];
            printf("gltf loader: texture with index %d, name '%s', and uri '%s'\n", i, img.name.c_str(), img.uri.c_str());
            _tx_map[i] = _rm.create_texture_from_data(img.name.c_str(), img.width, img.height, img.component, img.image.data());
        }
        return true;
    }

    bool parse_materials(const tinygltf::Model &model) {
        for (uint32_t i = 0; i < model.materials.size(); i++) {
            const tinygltf::Material &mtl = model.materials[i];
            printf("gltf loader: material with index %d, name '%s'\n", i, mtl.name.c_str());
            log_info("Material '%s'", mtl.name.c_str());
            log_info("  Emissive factor %f %f %f", mtl.emissiveFactor[0], mtl.emissiveFactor[1], mtl.emissiveFactor[2]);
            log_info("  Alpha Mode '%s'", mtl.alphaMode.c_str());
            log_info("  Alpha cutoff '%f'", mtl.alphaCutoff);
            log_info("  double-sided '%d'", (int)mtl.doubleSided);
            // _tx_map[i] = _rm.create_texture_from_data(img.name.c_str(), img.width, img.height, img.component, img.image.data());
        }
        return true;
    }

    std::vector<Mesh *> &meshes() { return _meshes; }

    std::string _filename = "";
    ResourceManager &_rm;
    std::unordered_map<uint32_t, Texture *> _tx_map;
    std::vector<Mesh *> _meshes;
};

} // namespace

namespace glengine {

std::vector<Mesh *> create_from_gltf(ResourceManager &rm, const char *filename) {
    MeshData md;
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    printf("loading gltf model: %s\n", filename);
    bool ret = false;
    if (get_file_extension(filename)=="gltf") {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    } else { // assume binary gltf (usually .glb)
        ret = loader.LoadBinaryFromFile(&model, &err, &warn, filename);
    }

    if (!warn.empty()) {
        printf("Warn: %s\n", warn.c_str());
    }
    if (!err.empty()) {
        printf("Err: %s\n", err.c_str());
    }
    if (!ret) {
        printf("Failed to parse glTF\n");
        return std::vector<Mesh *>();
    }

    printf("the model has %d buffers\n", (int)model.buffers.size());
    printf("the model has %d textures\n", (int)model.images.size());
    const tinygltf::Scene &scene = model.scenes[model.defaultScene];
    printf("the scene has %d nodes\n", (int)scene.nodes.size());
    GltfLoader ml(filename, rm);
    ml.load_textures(model);
    printf("loaded %d textures\n", (int)ml._tx_map.size());
    ml.parse_materials(model);
    // this loader makes the assumption that the entire scene is a single model, rendered with the same shader
    math::Matrix4f root_tf = math::matrix4_identity<float>();
    root_tf = math::create_transformation(
        {0, 0, 0}, math::quat_from_euler_321<float>(M_PI_2, 0, 0)); // because by default gltf are y-up
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        assert((scene.nodes[i] >= 0) && (scene.nodes[i] < int(model.nodes.size())));
        ml.load_node(model, model.nodes[scene.nodes[i]], root_tf);
    }
    return ml.meshes();
}

std::vector<Mesh *> ResourceManager::create_mesh_from_file(const char *filename) {
    return create_from_gltf(*this, filename);
}

} // namespace glengine
