/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#if defined(__TINYC__) || defined(__WATCOMC__)
#include <stdint.h>
/* Classic binary divide-and-conquer popcount.
   This is popcount_2() from
   http://en.wikipedia.org/wiki/Hamming_weight */
uint32_t popcount_2(uint32_t x){
    uint32_t m1 = 0x55555555;
    uint32_t m2 = 0x33333333;
    uint32_t m4 = 0x0f0f0f0f;
    x -= (x >> 1) & m1;
    x = (x & m2) + ((x >> 2) & m2);
    x = (x + (x >> 4)) & m4;
    x += x >>  8;
    return (x + (x >> 16)) & 0x3f;
}
uint64_t __builtin_popcountll(uint64_t x){
	return popcount_2(x) + popcount_2(x >> 32);
}
uint32_t __builtin_popcount(uint32_t x){
	return popcount_2(x);
}

void __sync_synchronize(){}
#endif
