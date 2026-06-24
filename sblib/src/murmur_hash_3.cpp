//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// origin https://github.com/aappleby/smhasher

#include "sblib/murmur_hash_3.h"


uint32_t rotl32(const uint32_t x, const int8_t r)
{
    return (x << r) | (x >> (32 - r));
}

uint64_t rotl64(const uint64_t x, const int8_t r)
{
    return (x << r) | (x >> (64 - r));
}

#define BIG_CONSTANT(x) (x##LLU)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

uint32_t getblock32(const uint32_t * p, const int32_t i)
{
    return p[i];
}

uint64_t getblock64(const uint64_t * p, const int64_t i)
{
    return p[i];
}

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

uint32_t fmix32(uint32_t h)
{
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h;
}

uint64_t fmix64(uint64_t k)
{
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xff51afd7ed558ccd);
    k ^= k >> 33;
    k *= BIG_CONSTANT(0xc4ceb9fe1a85ec53);
    k ^= k >> 33;
    return k;
}

void murmurHash3_x86_32(const void * key, const uint32_t len, const uint32_t seed, void * out)
{
    const auto data = static_cast<const uint8_t*>(key);
    const int32_t nBlocks = len / 4;

    uint32_t h1 = seed;

    constexpr uint32_t c1 = 0xcc9e2d51;
    constexpr uint32_t c2 = 0x1b873593;

    //----------
    // body

    const auto blocks = reinterpret_cast<const uint32_t *>(data + nBlocks * 4);

    for (int32_t i = -nBlocks; i; i++)
    {
        uint32_t k1 = getblock32(blocks, i);

        k1 *= c1;
        k1 = rotl32(k1, 15);
        k1 *= c2;

        h1 ^= k1;
        h1 = rotl32(h1, 13);
        h1 = h1 * 5 + 0xe6546b64;
    }

    //----------
    // tail

    const auto tail = (const uint8_t*)(data + nBlocks * 4);

    uint32_t k1 = 0;

    switch(len & 3)
    {
        case 3: k1 ^= tail[2] << 16;
                [[fallthrough]];
        case 2: k1 ^= tail[1] << 8;
                [[fallthrough]];
        case 1: k1 ^= tail[0];
              k1 *= c1;
              k1 = rotl32(k1, 15);
              k1 *= c2;
              h1 ^= k1;
              [[fallthrough]];
        default:
            ;
    }

    //----------
    // finalization

    h1 ^= len;

    h1 = fmix32(h1);

    *static_cast<uint32_t*>(out) = h1;
}

void murmurHash3_x86_128(const void * key, uint32_t len, uint32_t seed, void * out)
{
    auto data = static_cast<const uint8_t*>(key);
    const int32_t nBlocks = len / 16;

    uint32_t h1 = seed;
    uint32_t h2 = seed;
    uint32_t h3 = seed;
    uint32_t h4 = seed;

    constexpr uint32_t c1 = 0x239b961b;
    constexpr uint32_t c2 = 0xab0e9789;
    constexpr uint32_t c3 = 0x38b34ae5;
    constexpr uint32_t c4 = 0xa1e38b93;

    //----------
    // body

    auto blocks = reinterpret_cast<const uint32_t *>(data + nBlocks * 16);

    for(int32_t i = -nBlocks; i; i++)
    {
        uint32_t k1 = getblock32(blocks, i * 4 + 0);
        uint32_t k2 = getblock32(blocks, i * 4 + 1);
        uint32_t k3 = getblock32(blocks, i * 4 + 2);
        uint32_t k4 = getblock32(blocks, i * 4 + 3);

        k1 *= c1; k1  = rotl32(k1, 15); k1 *= c2; h1 ^= k1;

        h1 = rotl32(h1, 19); h1 += h2; h1 = h1 * 5 + 0x561ccd1b;

        k2 *= c2; k2  = rotl32(k2, 16); k2 *= c3; h2 ^= k2;

        h2 = rotl32(h2, 17); h2 += h3; h2 = h2 * 5 + 0x0bcaa747;

        k3 *= c3; k3  = rotl32(k3, 17); k3 *= c4; h3 ^= k3;

        h3 = rotl32(h3, 15); h3 += h4; h3 = h3 * 5 + 0x96cd1c35;

        k4 *= c4; k4  = rotl32(k4, 18); k4 *= c1; h4 ^= k4;

        h4 = rotl32(h4, 13); h4 += h1; h4 = h4 * 5 + 0x32ac3b17;
    }

    //----------
    // tail

    auto tail = (const uint8_t*)(data + nBlocks * 16);

    uint32_t k1 = 0;
    uint32_t k2 = 0;
    uint32_t k3 = 0;
    uint32_t k4 = 0;

    switch(len & 15)
    {
        case 15: k4 ^= tail[14] << 16;
                 [[fallthrough]];
        case 14: k4 ^= tail[13] << 8;
                 [[fallthrough]];
        case 13: k4 ^= tail[12] << 0;
                 k4 *= c4; k4 = rotl32(k4, 18); k4 *= c1; h4 ^= k4;
                 [[fallthrough]];
        case 12: k3 ^= tail[11] << 24;
                 [[fallthrough]];
        case 11: k3 ^= tail[10] << 16;
                 [[fallthrough]];
        case 10: k3 ^= tail[ 9] << 8;
                 [[fallthrough]];
        case  9: k3 ^= tail[ 8] << 0;
                 k3 *= c3; k3 = rotl32(k3, 17); k3 *= c4; h3 ^= k3;
                 [[fallthrough]];
        case  8: k2 ^= tail[ 7] << 24;
                 [[fallthrough]];
        case  7: k2 ^= tail[ 6] << 16;
                 [[fallthrough]];
        case  6: k2 ^= tail[ 5] << 8;
                 [[fallthrough]];
        case  5: k2 ^= tail[ 4] << 0;
                 k2 *= c2; k2 = rotl32(k2, 16); k2 *= c3; h2 ^= k2;
                 [[fallthrough]];
        case  4: k1 ^= tail[ 3] << 24;
                 [[fallthrough]];
        case  3: k1 ^= tail[ 2] << 16;
                 [[fallthrough]];
        case  2: k1 ^= tail[ 1] << 8;
                 [[fallthrough]];
        case  1: k1 ^= tail[ 0] << 0;
                 k1 *= c1; k1  = rotl32(k1, 15); k1 *= c2; h1 ^= k1;
                 [[fallthrough]];
        default:
            ;
    }

    //----------
    // finalization

    h1 ^= len; h2 ^= len; h3 ^= len; h4 ^= len;

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    h1 = fmix32(h1);
    h2 = fmix32(h2);
    h3 = fmix32(h3);
    h4 = fmix32(h4);

    h1 += h2; h1 += h3; h1 += h4;
    h2 += h1; h3 += h1; h4 += h1;

    static_cast<uint32_t*>(out)[0] = h1;
    static_cast<uint32_t*>(out)[1] = h2;
    static_cast<uint32_t*>(out)[2] = h3;
    static_cast<uint32_t*>(out)[3] = h4;
}
