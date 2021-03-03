#include "gl_engine.h"
#include "gl_resource_manager.h"
#include "gl_prefabs.h"
#include "gl_logger.h"
#include "gl_object.h"
#include "gl_mesh.h"
#include "gl_material_diffuse.h"
#include "gl_material_pbr.h"
#include "gl_material_pbr_ibl.h"

#include "math/vmath.h"
#include "stb/stb_image.h"
#include "tinygltf/tiny_gltf.h"

#include "sokol_gfx.h"

#include <set>

using namespace glengine;

namespace {

std::string get_file_extension(const std::string &filename) {
    if (filename.find_last_of(".") != std::string::npos)
        return filename.substr(filename.find_last_of(".") + 1);
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
            log_info("model rotation %f %f %f %f", float(r[0]), float(r[1]), float(r[2]), float(r[3]));
        }
        tf = math::create_translation(translation) * math::create_transformation({0.0f, 0.0f, 0.0f}, rotation) *
             math::create_scaling(scale);
    }
    return tf;
}

class GltfLoader {
  public:
    GltfLoader(const std::string &filename, GLEngine &eng)
    : _filename(filename)
    , _eng(eng)
    , _rm(eng.resource_manager()) {}

    bool load_mesh(const tinygltf::Model &model, const tinygltf::Mesh &mesh, const math::Matrix4f &tf) {
        for (size_t pi = 0; pi < mesh.primitives.size(); ++pi) {
            tinygltf::Primitive primitive = mesh.primitives[pi];
            // for (const auto & attr : primitive.attributes) {
            //     log_warning("prim attr: %s %d", attr.first.c_str(), attr.second);
            // }
            if (primitive.mode != TINYGLTF_MODE_TRIANGLES || primitive.attributes.count("POSITION") == 0) {
                printf("SKIP (only triangles are supported for now)!\n");
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
            // tangent
            math::Vector4f *tangents = nullptr;
            if (primitive.attributes.count("TANGENT") != 0) {
                const tinygltf::Accessor &tang_accessor = model.accessors[primitive.attributes["TANGENT"]];
                if (tang_accessor.type == TINYGLTF_TYPE_VEC3 ||
                    tang_accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
                    const tinygltf::BufferView &tang_bufferView = model.bufferViews[tang_accessor.bufferView];
                    const tinygltf::Buffer &tang_buffer = model.buffers[tang_bufferView.buffer];
                    tangents = (math::Vector4f *)(tang_buffer.data.data() + tang_bufferView.byteOffset +
                                                  tang_accessor.byteOffset);
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
            if (pos_accessor.count < 3) {
                printf("empty mesh\n");
                continue;
            }
            glengine::MeshData md;
            auto rot = tf;
            math::set_translation(rot, {0, 0, 0});
            for (uint32_t vi = 0; vi < pos_accessor.count; vi++) {
                auto pos = tf * positions[vi];
                auto norm = normals ? rot * normals[vi] : math::Vector3f{0.0f, 0.0f, 0.0f};
                auto tc0 = texcoords ? math::Vector2f{texcoords[vi].s, texcoords[vi].t} : math::Vector2f{0.0f, 0.0f};
                auto tang = tangents ? rot * tangents[vi] : math::Vector4f{0.0f, 0.0f, 0.0f, 1.0f};
                // printf("(%5.3f,%5.3f,%5.3f) ", tang.x, tang.y, tang.z);
                md.vertices.push_back({pos, {150, 150, 150, 255}, norm, tc0, {tang.x, tang.y, tang.z}});
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
            glengine::Mesh *mesh = _eng.create_mesh();
            mesh->init(md.vertices, md.indices);
            // material
            glengine::Material *material = nullptr;
            if (primitive.material >= 0) {
                const tinygltf::Material &mtl = model.materials[primitive.material];
                material = create_material(mtl);
            } else {
                material = _eng.create_material<glengine::MaterialDiffuse>(
                    SG_PRIMITIVETYPE_TRIANGLES, md.indices.size() > 0 ? SG_INDEXTYPE_UINT32 : SG_INDEXTYPE_NONE);
            }
            Renderable go{mesh, material};
            _renderables.push_back(go);
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
            log_debug("gltf loader: texture with index %d, name '%s', and uri '%s'", i, img.name.c_str(),
                      img.uri.c_str());
            // log_debug("image %d %d %d",img.component, img.bits, img.pixel_type);
            sg_image_desc img_desc = {0};
            img_desc.width = img.width;
            img_desc.height = img.height;
            img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
            img_desc.min_filter = SG_FILTER_LINEAR;
            img_desc.mag_filter = SG_FILTER_LINEAR;
            img_desc.data.subimage[0][0] = {
                .ptr = img.image.data(),
                .size = img.image.size(),
            };
            img_desc.label = img.uri.c_str();
            _tx_map[i] = sg_make_image(img_desc);
        }
        return true;
    }

    bool parse_materials(const tinygltf::Model &model, bool verbose = false) {
        for (uint32_t i = 0; i < model.materials.size(); i++) {
            const tinygltf::Material &mtl = model.materials[i];
            if (verbose) {
                log_info("Material '%s', index %d", mtl.name.c_str(), i);
                log_info("  Emissive factor %f %f %f", mtl.emissiveFactor[0], mtl.emissiveFactor[1],
                         mtl.emissiveFactor[2]);
                log_info("  emissive texture idx %d coords %d", mtl.emissiveTexture.index,
                         mtl.emissiveTexture.texCoord);
                log_info("  occlusion texture idx %d coords %d", mtl.occlusionTexture.index,
                         mtl.occlusionTexture.texCoord);
                log_info("  normal texture idx %d coords %d", mtl.normalTexture.index, mtl.normalTexture.texCoord);
                log_info("  Alpha Mode '%s'", mtl.alphaMode.c_str());
                log_info("  Alpha cutoff '%f'", mtl.alphaCutoff);
                log_info("  double-sided '%d'", (int)mtl.doubleSided);
                auto &pbr = mtl.pbrMetallicRoughness;
                log_info("  PBR:");
                log_info("    base color factor %f %f %f [%f]", pbr.baseColorFactor[0], pbr.baseColorFactor[1],
                         pbr.baseColorFactor[2], pbr.baseColorFactor[3]);
                log_info("    base color texture idx %d coords %d", pbr.baseColorTexture.index,
                         pbr.baseColorTexture.texCoord);
                log_info("    metallic factor %f", pbr.metallicFactor);
                log_info("    roughness factor %f", pbr.roughnessFactor);
                log_info("    metallic roughness texture idx %d coords %d", pbr.metallicRoughnessTexture.index,
                         pbr.metallicRoughnessTexture.texCoord);
            }
            // get_or_create_material(mtl);
        }
        return true;
    }

    std::string material_fullname(const tinygltf::Material &mtl) { return _filename + std::string("_") + mtl.name; }

    glengine::Material *create_material(const tinygltf::Material &mtl) {
        std::string mtl_name = material_fullname(mtl);
        bool is_unlit = false;
        for (auto &ext : mtl.extensions) {
            if (ext.first == "KHR_materials_unlit") {
                is_unlit = true;
                log_info("material %s is using KHR_materials_unlit extension - disabling PBR", mtl_name.c_str());
            }
        }
        auto &pbr = mtl.pbrMetallicRoughness;
        if (is_unlit) {
            if (pbr.baseColorTexture.index >= 0) {
                auto material = _eng.create_material<glengine::MaterialDiffuseTextured>(SG_PRIMITIVETYPE_TRIANGLES,
                                                                                        SG_INDEXTYPE_UINT32);
                material->tex_diffuse = _tx_map[pbr.baseColorTexture.index];
                return material;
            } else {
                auto material =
                    _eng.create_material<glengine::MaterialDiffuse>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
                material->color = {(uint8_t)(pbr.baseColorFactor[0] * 255), (uint8_t)(pbr.baseColorFactor[1] * 255),
                                   (uint8_t)(pbr.baseColorFactor[2] * 255), 255};
                return material;
            }
        } else {
            // PBR
            auto material =
                _eng.create_material<glengine::MaterialPBRIBL>(SG_PRIMITIVETYPE_TRIANGLES, SG_INDEXTYPE_UINT32);
            if (pbr.baseColorTexture.index >= 0) {
                material->tex_diffuse = _tx_map[pbr.baseColorTexture.index];
            }
            material->roughness_factor = pbr.roughnessFactor;
            material->metallic_factor = pbr.metallicFactor;
            if (pbr.metallicRoughnessTexture.index >= 0) {
                material->tex_metallic_roughness = _tx_map[pbr.metallicRoughnessTexture.index];
            }
            if (mtl.normalTexture.index >= 0) {
                material->tex_normal = _tx_map[mtl.normalTexture.index];
            }
            material->emissive_factor = {(float)mtl.emissiveFactor[0], (float)mtl.emissiveFactor[1],
                                         (float)mtl.emissiveFactor[2]};
            if (mtl.emissiveTexture.index >= 0) {
                material->tex_emissive = _tx_map[mtl.emissiveTexture.index];
            }
            if (mtl.occlusionTexture.index >= 0) {
                material->tex_occlusion = _tx_map[mtl.occlusionTexture.index];
            }
            return material;
        }
        return nullptr;
    }

    std::vector<Mesh *> &meshes() { return _meshes; }
    std::vector<Renderable> &renderables() { return _renderables; }

    std::string _filename = "";
    GLEngine &_eng;
    ResourceManager &_rm;
    std::unordered_map<uint32_t, sg_image> _tx_map;
    std::vector<Mesh *> _meshes;
    std::vector<Renderable> _renderables;
};

} // namespace

namespace glengine {

std::vector<Renderable> create_from_gltf(GLEngine &eng, const char *filename) {
    MeshData md;
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    printf("loading gltf model: %s\n", filename);
    bool ret = false;
    if (get_file_extension(filename) == "gltf") {
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
        return std::vector<Renderable>();
    }

    log_debug("the model has %d buffers\n", (int)model.buffers.size());
    log_debug("the model has %d textures\n", (int)model.images.size());
    const tinygltf::Scene &scene = model.scenes[model.defaultScene];
    log_debug("the scene has %d nodes\n", (int)scene.nodes.size());
    GltfLoader ml(filename, eng);
    ml.load_textures(model);
    log_debug("loaded %d textures\n", (int)ml._tx_map.size());
    ml.parse_materials(model, true);
    // this loader makes the assumption that the entire scene is a single model
    math::Matrix4f root_tf = math::matrix4_identity<float>();
    // root_tf = math::create_transformation(
    //     {0, 0, 0}, math::quat_from_euler_321<float>(M_PI_2, 0, 0)); // because by default gltf are y-up
    // root_tf = math::matrix4_identity<float>();
    for (size_t i = 0; i < scene.nodes.size(); ++i) {
        assert((scene.nodes[i] >= 0) && (scene.nodes[i] < int(model.nodes.size())));
        ml.load_node(model, model.nodes[scene.nodes[i]], root_tf);
    }
    return ml.renderables();
}

} // namespace glengine
