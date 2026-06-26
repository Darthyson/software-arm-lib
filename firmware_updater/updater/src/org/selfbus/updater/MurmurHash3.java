package org.selfbus.updater;

// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// origin https://github.com/aappleby/smhasher


// Java port of the x86_32 variant.
public class MurmurHash3 {

    private static int rotl32(final int x, final int r) {
        return (x << r) | (x >>> (32 - r));
    }

    private static int getblock32(final byte[] data, final int offset) {
        return (data[offset]     & 0xFF)
             | (data[offset + 1] & 0xFF) << 8
             | (data[offset + 2] & 0xFF) << 16
             | (data[offset + 3] & 0xFF) << 24;
    }

    private static int fmix32(int h) {
        h ^= h >>> 16;
        h *= 0x85ebca6b;
        h ^= h >>> 13;
        h *= 0xc2b2ae35;
        h ^= h >>> 16;
        return h;
    }

    /**
     * Computes the MurmurHash3 x86_32 hash of the given byte array.
     *
     * @param key  input data
     * @param seed seed value
     * @return 32-bit hash as a long (unsigned interpretation) or use as int
     */
    @SuppressWarnings("fallthrough")
    public static int murmurHash3_x86_32(final byte[] key, final int seed) {
        final int len = key.length;
        final int nBlocks = len / 4;

        int h1 = seed;

        final int c1 = 0xcc9e2d51;
        final int c2 = 0x1b873593;

        // body
        for (int i = 0; i < nBlocks; i++) {
            int k1 = getblock32(key, i * 4);

            k1 *= c1;
            k1 = rotl32(k1, 15);
            k1 *= c2;

            h1 ^= k1;
            h1 = rotl32(h1, 13);
            h1 = h1 * 5 + 0xe6546b64;
        }

        // tail
        int k1 = 0;
        final int tailOffset = nBlocks * 4;

        switch (len & 3) {
            case 3: k1 ^= (key[tailOffset + 2] & 0xFF) << 16;
                    // fall through
            case 2: k1 ^= (key[tailOffset + 1] & 0xFF) << 8;
                    // fall through
            case 1: k1 ^= (key[tailOffset] & 0xFF);
                    k1 *= c1;
                    k1 = rotl32(k1, 15);
                    k1 *= c2;
                    h1 ^= k1;
                    break;
            default:
                break;
        }

        // finalization
        h1 ^= len;
        h1 = fmix32(h1);

        return h1;
    }
}
