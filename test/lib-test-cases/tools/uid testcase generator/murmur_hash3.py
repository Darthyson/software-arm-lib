"""Python port of MurmurHash3 x86_32 by Austin Appleby (public domain).

Origin: https://github.com/aappleby/smhasher
"""

import struct


def _rotl32(x: int, r: int) -> int:
    return ((x << r) | (x >> (32 - r))) & 0xFFFFFFFF


def _fmix32(h: int) -> int:
    h = (h ^ (h >> 16)) & 0xFFFFFFFF
    h = (h * 0x85ebca6b) & 0xFFFFFFFF
    h = (h ^ (h >> 13)) & 0xFFFFFFFF
    h = (h * 0xc2b2ae35) & 0xFFFFFFFF
    h = (h ^ (h >> 16)) & 0xFFFFFFFF
    return h


def murmur_hash3_x86_32(key: bytes, seed: int = 0) -> bytes:
    """Return the MurmurHash3 x86_32 hash of *key* as 4 little-endian bytes."""
    length = len(key)
    n_blocks = length // 4

    h1 = seed & 0xFFFFFFFF
    c1 = 0xcc9e2d51
    c2 = 0x1b873593

    # Body: process 4-byte blocks
    for i in range(n_blocks):
        k1 = struct.unpack_from('<I', key, i * 4)[0]
        k1 = (k1 * c1) & 0xFFFFFFFF
        k1 = _rotl32(k1, 15)
        k1 = (k1 * c2) & 0xFFFFFFFF
        h1 ^= k1
        h1 = _rotl32(h1, 13)
        h1 = (h1 * 5 + 0xe6546b64) & 0xFFFFFFFF

    # Tail: remaining 1-3 bytes (mirrors the C++ switch fallthrough)
    tail = n_blocks * 4
    k1 = 0
    remainder = length & 3

    if remainder >= 3:
        k1 ^= key[tail + 2] << 16
    if remainder >= 2:
        k1 ^= key[tail + 1] << 8
    if remainder >= 1:
        k1 ^= key[tail]
        k1 = (k1 * c1) & 0xFFFFFFFF
        k1 = _rotl32(k1, 15)
        k1 = (k1 * c2) & 0xFFFFFFFF
        h1 ^= k1

    # Finalization
    h1 ^= length
    h1 = _fmix32(h1)

    return struct.pack('<I', h1)


if __name__ == '__main__':
    # Quick sanity checks against known MurmurHash3 reference values
    assert murmur_hash3_x86_32(b'', seed=0) == struct.pack('<I', 0x00000000)
    assert murmur_hash3_x86_32(b'', seed=1) == struct.pack('<I', 0x514E28B7)
    assert murmur_hash3_x86_32(b'', seed=0xFFFFFFFF) == struct.pack('<I', 0x81F16F39)
    assert murmur_hash3_x86_32(b'\xff\xff\xff\xff', seed=0) == struct.pack('<I', 0x76293B50)
    assert murmur_hash3_x86_32(b'\x21\x43\x65\x87', seed=0) == struct.pack('<I', 0xF55B516B)
    assert murmur_hash3_x86_32(b'\x21\x43\x65\x87', seed=0x5082EDEE) == struct.pack('<I', 0x2362F9DE)
    assert murmur_hash3_x86_32(b'\x21\x43\x65', seed=0) == struct.pack('<I', 0x7E4A8634)
    assert murmur_hash3_x86_32(b'\x21\x43', seed=0) == struct.pack('<I', 0xA0F7B07A)
    assert murmur_hash3_x86_32(b'\x21', seed=0) == struct.pack('<I', 0x72661CF4)
    print('All self-tests passed.')
