/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include <time.h>

#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#if false
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
#endif

#if defined(_MSC_VER) || defined(__TINYC__)
void __sync_synchronize() {}
#endif

#if defined(_MSC_VER)
	#include "windows.h"
#else
	#include <sys/time.h>
#endif

#ifdef __MINGW32__
	#include <pthread_time.h>
#endif

/* Return monotonic time in milliseconds */
u64 getMSecs(){
#ifdef _MSC_VER
	return GetTickCount64();
#else
	struct timespec tv;
	clock_gettime(CLOCK_MONOTONIC,&tv);
	return (tv.tv_nsec / 1000000) + (tv.tv_sec * 1000);
#endif
}

/* Add all the platform specific constants to C */
void lAddPlatformVars(lClosure *c){
	lVal valOS, valArch;
	#if defined(__HAIKU__)
	valOS = lValSym("Haiku");
	#elif defined(__APPLE__)
	valOS =  lValSym("MacOS");
	#elif defined(__EMSCRIPTEN__)
	valOS =  lValSym("Emscripten");
	#elif defined(__MINGW32__)
	valOS =  lValSym("Windows");
	#elif defined(__MSYS__)
	valOS = lValSym("Legacy Windows");
	#elif defined(__MINIX__) || defined(__MINIX3__)
	valOS = lValSym("Minix");
	#elif defined(__linux__)
		#if defined(__UCLIBC__)
		valOS = lValSym("uClibc/Linux");
		#elif defined(__GLIBC__)
		valOS = lValSym("GNU/Linux");
		#elif defined(__BIONIC__)
		valOS = lValSym("Android");
		#else
		valOS = lValSym("musl?/Linux");
		#endif
	#elif defined(__FreeBSD__)
	valOS = lValSym("FreeBSD");
	#elif defined(__OpenBSD__)
	valOS = lValSym("OpenBSD");
	#elif defined(__NetBSD__)
	valOS = lValSym("NetBSD");
	#elif defined(__DragonFly__)
	valOS = lValSym("DragonFlyBSD");
	#else
	valOS = lValSym("Unknown");
	#endif
	lDefineVal(c, "System/OS", valOS);

	#if defined(__arm__)
	valArch = lValSym("armv7l");
	#elif defined(__aarch64__)
	valArch = lValSym("aarch64");
	#elif defined(__x86_64__) || defined(__amd64__)
	valArch = lValSym("x86_64");
	#elif defined(__i386__)
	valArch = lValSym("x86");
	#elif defined(__EMSCRIPTEN__)
	valArch = lValSym("wasm");
	#elif defined(__powerpc__)
	valArch = lValSym("powerpc");
	#else
	valArch = lValSym("unknown");
	#endif
	lDefineVal(c, "System/Architecture", valArch);
}
