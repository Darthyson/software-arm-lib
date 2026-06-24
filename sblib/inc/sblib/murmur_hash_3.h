//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// origin https://github.com/aappleby/smhasher

#ifndef SBLIB_MURMURHASH3_H_
#define SBLIB_MURMURHASH3_H_

#include <cstdint>


void murmurHash3_x86_32(const void * key, uint32_t len, uint32_t seed, void * out);

#endif /* SBLIB_MURMURHASH3_H_ */
