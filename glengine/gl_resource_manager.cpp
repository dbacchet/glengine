#include "gl_resource_manager.h"
#include "gl_utils.h"
#include "gl_logger.h"
#include "gl_material.h"
#include "gl_mesh.h"

#include "stb/stb_image.h"
#include "stb/stb_image_resize.h"

namespace {

// generate mipmaps for all the possible levels
int generate_mipmaps(uint8_t *mip_levels[SG_MAX_MIPMAPS], int img_width, int img_height, int channels,
                     sg_image_desc &img_desc) {
    int level = 1;
    for (level = 1; level < SG_MAX_MIPMAPS; level++) {
        int w = img_width / (1 << level);
        int h = img_height / (1 << level);
        if (w < 1 || h < 1) {
            break;
        }
        mip_levels[level] = (uint8_t *)malloc(w * h * channels);
        printf("generate mipmap level %d with resolution %dx%d\n", level, w, h);
        // resize image (starting from the original image). Use explicit call, setting all options
        if (!stbir_resize_uint8_generic(mip_levels[0], img_width, img_height, 0, // input image data
                                        mip_levels[level], w, h, 0,              // output image data
                                        channels, channels == 3 ? STBIR_ALPHA_CHANNEL_NONE : 3, 0, STBIR_EDGE_CLAMP,
                                        STBIR_FILTER_BOX, STBIR_COLORSPACE_SRGB, 0)) {
            printf("Error resizing image to %dx%d\n", w, h);
            break;
        }
        img_desc.content.subimage[0][level] = {
            .ptr = mip_levels[level],
            .size = w * h * channels,
        };
    }
    img_desc.num_mipmaps = level;
    // img_desc.min_lod = 0.0f;
    // img_desc.max_lod = 9.0f;
    img_desc.min_filter = SG_FILTER_LINEAR_MIPMAP_NEAREST;
    img_desc.min_filter = SG_FILTER_LINEAR_MIPMAP_LINEAR;
    printf("generated %d mipmap levels\n", level);
    printf("generated %d mipmap levels\n", img_desc.num_mipmaps);

    return level;
}

} // namespace
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
    for (int i = 0; i < DefaultImageNum; i++) {
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

sg_image ResourceManager::get_or_create_image(const char *filename, bool gen_mipmaps) {
    int img_width, img_height, num_channels;
    const int desired_channels = 4;
    stbi_set_flip_vertically_on_load(true);
    uint8_t *mip_levels[SG_MAX_MIPMAPS] = {0};
    mip_levels[0] = stbi_load(filename, &img_width, &img_height, &num_channels, desired_channels);
    if (mip_levels[0]) {
        sg_image_desc img_desc = {0};
        img_desc.width = img_width;
        img_desc.height = img_height;
        img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        img_desc.min_filter = SG_FILTER_LINEAR;
        img_desc.mag_filter = SG_FILTER_LINEAR;
        img_desc.content.subimage[0][0] = {
            .ptr = mip_levels[0],
            .size = img_width * img_height * desired_channels,
        };
        img_desc.label = filename;
        // generate mipmaps
        if (gen_mipmaps) {
            generate_mipmaps(mip_levels, img_width, img_height, desired_channels, img_desc);
        }
        img_desc.max_anisotropy = 4;
        sg_image img = get_or_create_image(img_desc);
        for (int i = 0; i < SG_MAX_MIPMAPS; i++) {
            if (mip_levels[i]) {
                stbi_image_free(mip_levels[i]);
            }
        }
        return img;
    }
    return {SG_INVALID_ID};
}

sg_image ResourceManager::get_or_create_image(const uint8_t *data, int32_t len, bool gen_mipmaps) {
    int img_width, img_height, num_channels;
    const int desired_channels = 4;
    stbi_set_flip_vertically_on_load(true);
    uint8_t *mip_levels[SG_MAX_MIPMAPS] = {0};
    mip_levels[0] = stbi_load_from_memory(data, len, &img_width, &img_height, &num_channels, desired_channels);
    if (mip_levels[0]) {
        char label[32];
        sprintf(label, "ptr:%p len:%d", data, len);
        sg_image_desc img_desc = {0};
        img_desc.width = img_width;
        img_desc.height = img_height;
        img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        img_desc.min_filter = SG_FILTER_LINEAR;
        img_desc.mag_filter = SG_FILTER_LINEAR;
        img_desc.content.subimage[0][0] = {
            .ptr = mip_levels[0],
            .size = img_width * img_height * desired_channels,
        };
        img_desc.label = label;
        // generate mipmaps
        if (gen_mipmaps) {
            generate_mipmaps(mip_levels, img_width, img_height, desired_channels, img_desc);
        }
        img_desc.max_anisotropy = 4;
        sg_image img = get_or_create_image(img_desc);
        for (int i = 0; i < SG_MAX_MIPMAPS; i++) {
            if (mip_levels[i]) {
                stbi_image_free(mip_levels[i]);
            }
        }
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
