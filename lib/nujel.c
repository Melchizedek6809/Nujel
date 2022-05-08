/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"

#include "misc/pf.h"
#include "exception.h"
#include "allocation/allocator.h"
#include "allocation/symbol.h"
#include "collection/list.h"
#include "operation.h"
#include "reader.h"
#include "type/closure.h"
#include "type/symbol.h"
#include "vm/eval.h"

extern u8 stdlib_no_data[];
bool lVerbose = false;

/* Initialize the allocator and symbol table, needs to be called before as
 * soon as possible, since most procedures depend on it.*/
void lInit(){
	lSymbolInit();
}

/* Evaluate the Nujel Lambda expression and return the results */
lVal *lLambda(lClosure *c, lVal *args, lVal *lambda){
	const int SP = lRootsGet();
	lClosure *tmpc = lClosureNewFunCall(c, args, lambda);
	lVal *ret = lBytecodeEval(tmpc, NULL, &lambda->vClosure->text->vBytecodeArr, false);
	lRootsRet(SP);
	return ret;
}

/* Run fun with args, evaluating args if necessary  */
lVal *lApply(lClosure *c, lVal *args, lVal *fun){
	switch(fun ? fun->type : ltNoAlloc){
	case ltLambda:
		return lLambda(c,args,fun);
	case ltNativeFunc:
		return fun->vNFunc->fp(c,args);
	case ltObject:
		if(args && args->type == ltBytecodeArr){
			RCP(c);
			return lBytecodeEval(fun->vClosure, NULL, &args->vBytecodeArr, false);
		} /* fall-through */
	default:
		lExceptionThrowValClo("type-error", "Can't apply to following val", fun, c);
		return NULL;
	}
}

/* Add all the platform specific constants to C */
static void lAddPlatformVars(lClosure *c){
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

/* Create a new root closure with the stdlib */
lClosure *lNewRoot(){
	lClosure *c = lClosureAlloc();
	c->parent = NULL;
	c->type = closureLet;
	lRootsClosurePush(c);
	lAddCoreFuncs(c);
	lAddPlatformVars(c);
	return lLoad(c, (const char *)stdlib_no_data);
}

/* Reads EXPR which should contain bytecode arrays and then evaluate them in C.
 * Mainly used for bootstrapping the stdlib and compiler out of precompiled .no
 * files. */
lClosure *lLoad(lClosure *c, const char *expr){
	const int RSP = lRootsGet();
	lVal *v = RVP(lRead(expr));
	for(lVal *n=v; n && n->type == ltPair; n = n->vList.cdr){
		const int RSSP = lRootsGet();
		lVal *car = n->vList.car;
		if((car == NULL) || (car->type != ltBytecodeArr)){
			lExceptionThrowValClo("load-error", "Can only load values of type :bytecode-arr", car, c);
		}
		lBytecodeEval(c, NULL, &car->vBytecodeArr, false);
		lRootsRet(RSSP);
	}
	lRootsRet(RSP);
	return c;
}
