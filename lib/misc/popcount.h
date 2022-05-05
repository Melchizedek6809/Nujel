#ifndef NUJEL_LIB_MISC_POPCOUNT
#define NUJEL_LIB_MISC_POPCOUNT

#if defined(__TINYC__) || defined(__WATCOMC__)
/* Classic binary divide-and-conquer popcount.
   This is popcount_2() from
   http://en.wikipedia.org/wiki/Hamming_weight */
static inline uint32_t popcount_2(uint32_t x){
    uint32_t m1 = 0x55555555;
    uint32_t m2 = 0x33333333;
    uint32_t m4 = 0x0f0f0f0f;
    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    x += x >>  8;
    return (x + (x >> 16)) & 0x3f;
}
static inline __builtin_popcountll(uint64_t x){
	return popcount_2(x) + popcount_2(x >> 32);
}
#endif

#endif
