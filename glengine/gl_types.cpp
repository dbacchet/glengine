#include "gl_types.h"

#include "math/vmath.h"
#include <cstdint>
#include <limits>

namespace glengine {

// This is a variant of MurmurHash2 modified to use the Merkle-Damgard
// construction. Bulk speed should be identical to Murmur2, small-key speed
// will be 10%-20% slower due to the added overhead at the end of the hash.
// This variant fixes a minor issue where null keys were more likely to
// collide with each other than expected, and also makes the function
// more amenable to incremental implementations.
#define mmix(h, k)                                                                                                     \
    {                                                                                                                  \
        k *= m;                                                                                                        \
        k ^= k >> r;                                                                                                   \
        k *= m;                                                                                                        \
        h *= m;                                                                                                        \
        h ^= k;                                                                                                        \
    }
uint32_t murmur_hash2_32(const void *key, int len, uint32_t seed) {
    const uint32_t m = 0x5bd1e995;
    const int32_t r = 24;
    uint32_t l = len;
    const uint8_t *data = (const uint8_t *)key;
    uint32_t h = seed;

    while (len >= 4) {
        // use this snippet in case the host platform is big-endian
        // uint32_t k = *(uint32_t *)data;
        // char *p = (char *)&k;
        // char c;
        // c = p[0];
        // p[0] = p[3];
        // p[3] = c;
        // c = p[1];
        // p[1] = p[2];
        // p[2] = c;
        uint32_t k = *(uint32_t *)data;
        mmix(h, k);
        data += 4;
        len -= 4;
    }

    uint32_t t = 0;
    switch (len) {
    case 3:
        t ^= data[2] << 16;
    case 2:
        t ^= data[1] << 8;
    case 1:
        t ^= data[0];
    };

    mmix(h, t);
    mmix(h, l);
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

uint32_t murmur_hash2_32(const char *str, uint32_t seed) {
    return murmur_hash2_32((void *)str, strlen(str), seed);
}

} // namespace glengine
