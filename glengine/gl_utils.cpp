#include "gl_utils.h"
#include "gl_object.h"
#include "gl_mesh.h"

#include <cstdint>
#include <limits>

namespace glengine {
uint32_t murmur_hash2_32(const void *key, int len, uint32_t seed) {
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.
    const uint32_t m = 0x5bd1e995;
    const int r = 24;
    // Initialize the hash to a 'random' value
    uint32_t h = seed ^ len;
    // Mix 4 bytes at a time into the hash
    const unsigned char *data = (const unsigned char *)key;
    while (len >= 4) {
        uint32_t k = *(uint32_t *)data;
        k *= m;
        k ^= k >> r;
        k *= m;
        h *= m;
        h ^= k;
        data += 4;
        len -= 4;
    }
    // Handle the last few bytes of the input array
    switch (len) {
    case 3:
        h ^= data[2] << 16;
    case 2:
        h ^= data[1] << 8;
    case 1:
        h ^= data[0];
        h *= m;
    };
    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

uint64_t murmur_hash2_64(const void *key, int len, uint64_t seed) {
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;
    uint64_t h = seed ^ (len * m);
    const uint64_t *data = (const uint64_t *)key;
    const uint64_t *end = data + (len / 8);
    while (data != end) {
        uint64_t k = *data++;
        k *= m;
        k ^= k >> r;
        k *= m;
        h ^= k;
        h *= m;
    }
    const unsigned char *data2 = (const unsigned char *)data;
    switch (len & 7) {
    case 7:
        h ^= uint64_t(data2[6]) << 48;
    case 6:
        h ^= uint64_t(data2[5]) << 40;
    case 5:
        h ^= uint64_t(data2[4]) << 32;
    case 4:
        h ^= uint64_t(data2[3]) << 24;
    case 3:
        h ^= uint64_t(data2[2]) << 16;
    case 2:
        h ^= uint64_t(data2[1]) << 8;
    case 1:
        h ^= uint64_t(data2[0]);
        h *= m;
    };
    h ^= h >> r;
    h *= m;
    h ^= h >> r;
    return h;
}

// struct Extents {
//     math::Vector3f bl;
//     math::Vector3f tr;
// };
void calc_object_extents(const glengine::Object *obj, bool with_children, math::Vector3f &bl, math::Vector3f &tr) {
    for (const auto &r : obj->_renderables) {
        const auto m = r.mesh;
        if (m) {
            for (const auto &v : m->vertices) {
                bl.x = std::min(bl.x, v.pos.x);
                bl.y = std::min(bl.y, v.pos.y);
                bl.z = std::min(bl.z, v.pos.z);
                tr.x = std::max(tr.x, v.pos.x);
                tr.y = std::max(tr.y, v.pos.y);
                tr.z = std::max(tr.z, v.pos.z);
            }
        }
        // recurse into children
        if (with_children) {
            for (const auto &c : obj->_children) {
                calc_object_extents(c, with_children, bl, tr);
            }
        }
    }
}

// return the bounding box of this object
AABB calc_bounding_box(const glengine::Object *obj, bool include_children) {
    math::Vector3f bl = {std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
                         std::numeric_limits<float>::max()}; // bottom left
    math::Vector3f tr = {std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
                         std::numeric_limits<float>::lowest()}; // top right
    calc_object_extents(obj, include_children, bl, tr);
    return AABB{(tr+bl)/2.0f, tr-bl};
}

} // namespace glengine
