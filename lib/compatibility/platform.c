/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "platform.h"
#include "../type/closure.h"
#include "../type/val.h"

/* Add all the platform specific constants to C */
void lAddPlatformVars(lClosure *c){
	lVal *valOS, *valArch;
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
	#elif defined(__WATCOMC__)
	valOS = lValSym("DOS");
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
	#elif defined(__WATCOMC__)
	valArch = lValSym("x86");
	#else
	valArch = lValSym("unknown");
	#endif
	lDefineVal(c, "System/Architecture", valArch);
}
