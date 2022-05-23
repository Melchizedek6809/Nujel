#ifndef NUJEL_LIB_MISC_POPCOUNT
#define NUJEL_LIB_MISC_POPCOUNT

#if defined(__TINYC__) || defined(__WATCOMC__)
uint32_t __builtin_popcount(uint32_t x);
uint64_t __builtin_popcountll(uint64_t x);
#endif

#endif
