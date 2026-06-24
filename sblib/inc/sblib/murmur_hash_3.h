//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// origin https://github.com/aappleby/smhasher
// online MurmurHash3 calculator https://www.codertools.net/tools/murmurhash.php

#ifndef SBLIB_MURMURHASH3_H_
#define SBLIB_MURMURHASH3_H_

#include <cstdint>


/**
 * @brief Computes the MurmurHash3 x86 32-bit hash of the given key.
 * 
 * @param key Pointer to the data to be hashed
 * @param len Length of the data in bytes
 * @param seed Seed value for the hash function (default 0)
 * @param out Pointer to the output buffer (must be at least 4 bytes)
 */
void murmurHash3_x86_32(const void * key, uint32_t len, uint32_t seed, void * out);

#endif /* SBLIB_MURMURHASH3_H_ */
