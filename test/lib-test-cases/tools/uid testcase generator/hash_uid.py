"""
uid_testcase_generator.py

Python equivalent of the C++ hashUID function:

    int hashUID(uint8_t* uid, const int len_uid,
                uint8_t* hash, const int len_hash)

The UID bytes are split into two halves, each interpreted as a
little-endian uint64, reduced modulo BigPrime48, XOR'd together,
and the requested number of low bytes returned.
"""
from __future__ import annotations

_MAX_HASH_WIDE = 16
_BIG_PRIME_48 = 281474976710597  # 0x0000_FFFF_FFFF_FFC5


def hash_uid(uid: bytes | bytearray, len_hash: int) -> bytes | None:
    """Hash a UID byte sequence.

    Args:
        uid:      UID bytes (1–16 bytes).
        len_hash: Number of hash bytes to return (1–len(uid)).

    Returns:
        ``bytes`` of length *len_hash* on success, ``None`` on invalid input.
    """
    len_uid = len(uid)

    if len_uid <= 0 or len_uid > _MAX_HASH_WIDE:
        return None
    if len_hash <= 0 or len_hash > len_uid:
        return None

    mid = len_uid // 2

    # Replicate memcpy into zero-initialized uint64 (little-endian platform)
    a = int.from_bytes(uid[:mid], byteorder="little")
    b = int.from_bytes(uid[mid:len_uid], byteorder="little")

    a = a % _BIG_PRIME_48
    b = b % _BIG_PRIME_48
    a = a ^ b

    return a.to_bytes(8, byteorder="little")[:len_hash]
