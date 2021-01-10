#pragma once

#include "math/vmath_types.h"

#include <cstdint>

namespace glengine {

/// hash function taken from the MurmurHash2 implementation at
/// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash2.h
uint32_t murmur_hash2_32(const void *key, int len, uint32_t seed);

/// hash function taken from the MurmurHash64A implementation at
/// https://github.com/aappleby/smhasher/blob/master/src/MurmurHash2.h
uint64_t murmur_hash2_64(const void *key, int len, uint64_t seed);

struct AABB {
    math::Vector3f center;
    math::Vector3f size;
};

class Object;

/// return the bounding box of this object
AABB calc_bounding_box(const glengine::Object *obj, bool include_children);

} // namespace glengine
