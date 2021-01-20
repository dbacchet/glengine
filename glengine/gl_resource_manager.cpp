#include "gl_resource_manager.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_material.h"
#include "gl_mesh.h"

#include "stb/stb_image.h"

namespace glengine {

ResourceManager::~ResourceManager() {
    // nothing to be done here
}
void ResourceManager::init() {
    log_debug("ResourceManager: init");
    // create placeholder textures
    log_debug("ResourceManager: create default textures");
    uint32_t pixels[64];
    sg_image_desc img = {
        .width = 8,
        .height = 8,
        .pixel_format = SG_PIXELFORMAT_RGBA8,
    };
    img.content.subimage[0][0] = {.ptr = pixels, .size = sizeof(pixels)};
    // white
    for (int i = 0; i < 64; i++) {
        pixels[i] = 0xFFFFFFFF;
    }
    _default_images[White] = sg_make_image(img);
    // black
    for (int i = 0; i < 64; i++) {
        pixels[i] = 0xFF000000;
    }
    _default_images[Black] = sg_make_image(img);
    // normal
    for (int i = 0; i < 64; i++) {
        pixels[i] = 0xFFFF8080;
    }
    _default_images[Normal] = sg_make_image(img);
}

void ResourceManager::terminate() {
    // cleanup meshes
    log_info("ResourceManager: cleanup meshes");
    for (auto &mesh : _meshes) {
        log_debug("Destroying mesh %p", &mesh);
        delete mesh;
    }
    // cleanup materials
    log_info("ResourceManager: cleanup materials");
    for (auto &mtl : _materials) {
        log_debug("Destroying material %p", &mtl);
        delete mtl;
    }
    _materials.clear();
    // cleanup image resources
    log_info("ResourceManager: cleanup images");
    for (auto it : _images) {
        log_debug("Destroying image %u", it.second.id);
        sg_destroy_image(it.second);
    }
    for (int i=0; i<DefaultImageNum; i++) {
        log_debug("Destroying (default) image %u", _default_images[i].id);
        sg_destroy_image(_default_images[i]);
    }
    _images.clear();
    // cleanup pipeline resources
    log_info("ResourceManager: cleanup pipelines");
    for (auto it : _pipelines) {
        log_debug("Destroying pipeline %u", it.second.id);
        sg_destroy_pipeline(it.second);
    }
    _pipelines.clear();
    // cleanup shader resources
    log_info("ResourceManager: cleanup shaders");
    for (auto it : _shaders) {
        log_debug("Destroying shader %u", it.second.id);
        sg_destroy_shader(it.second);
    }
    _shaders.clear();
}

sg_image ResourceManager::get_or_create_image(const sg_image_desc &desc) {
    uint64_t image_hash = murmur_hash2_64(&desc, sizeof(desc), 12345678);
    if (_images.count(image_hash) > 0) {
        return _images[image_hash];
    }
    // create the image and add it to the cache
    log_info("Creating image %s", desc.label);
    sg_image img = sg_make_image(desc);
    log_info("Created image %u", img.id);
    _images[image_hash] = img;
    return img;
}

sg_image ResourceManager::get_or_create_image(const char *filename) {
    int img_width, img_height, num_channels;
    const int desired_channels = 4;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc *pixels = stbi_load(filename, &img_width, &img_height, &num_channels, desired_channels);
    if (pixels) {
        sg_image_desc img_desc = {0};
        img_desc.width = img_width;
        img_desc.height = img_height;
        img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        img_desc.min_filter = SG_FILTER_LINEAR;
        img_desc.mag_filter = SG_FILTER_LINEAR;
        img_desc.content.subimage[0][0] = {
            .ptr = pixels,
            .size = img_width * img_height * 4,
        };
        img_desc.label = filename;
        sg_image img = get_or_create_image(img_desc);
        stbi_image_free(pixels);
        return img;
    }
    return {SG_INVALID_ID};
}

sg_image ResourceManager::get_or_create_image(const uint8_t *data, int32_t len) {
    int img_width, img_height, num_channels;
    const int desired_channels = 4;
    stbi_set_flip_vertically_on_load(true);
    stbi_uc *pixels = stbi_load_from_memory(data, len, &img_width, &img_height, &num_channels, desired_channels);
    if (pixels) {
        char label[32];
        sprintf(label, "ptr:%p len:%d", data, len);
        sg_image_desc img_desc = {0};
        img_desc.width = img_width;
        img_desc.height = img_height;
        img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        img_desc.min_filter = SG_FILTER_LINEAR;
        img_desc.mag_filter = SG_FILTER_LINEAR;
        img_desc.content.subimage[0][0] = {
            .ptr = pixels,
            .size = img_width * img_height * 4,
        };
        img_desc.label = label;
        sg_image img = get_or_create_image(img_desc);
        stbi_image_free(pixels);
        return img;
    }
    return {SG_INVALID_ID};
}

sg_image ResourceManager::default_image(DefaultImage type) {
    return _default_images[type];
}

sg_shader ResourceManager::get_or_create_shader(const sg_shader_desc &desc) {
    uint64_t shader_hash = murmur_hash2_64(&desc, sizeof(desc), 12345678);
    if (_shaders.count(shader_hash) > 0) {
        return _shaders[shader_hash];
    }
    // create the shader and add it to the cache
    log_info("Creating shader %s", desc.label);
    sg_shader shd = sg_make_shader(desc);
    log_info("Created shader %u", shd.id);
    _shaders[shader_hash] = shd;
    return shd;
}

sg_pipeline ResourceManager::get_or_create_pipeline(const sg_pipeline_desc &desc) {
    uint64_t pipeline_hash = murmur_hash2_64(&desc, sizeof(desc), 12345678);
    if (_pipelines.count(pipeline_hash) > 0) {
        return _pipelines[pipeline_hash];
    }
    // create the pipeline and add it to the cache
    log_info("Creating pipeline [%s]", desc.label);
    sg_pipeline pip = sg_make_pipeline(desc);
    log_info("Created pipeline %u", pip.id);
    _pipelines[pipeline_hash] = pip;
    return pip;
}

void ResourceManager::register_material(Material *mtl) {
    _materials.insert(mtl);
}

void ResourceManager::register_mesh(Mesh *msh) {
    _meshes.insert(msh);
}

} // namespace glengine
