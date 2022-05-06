/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"

#include "misc/pf.h"
#include "exception.h"
#include "allocation/native-function.h"
#include "allocation/tree.h"
#include "allocation/symbol.h"
#include "collection/list.h"
#include "operation.h"
#include "s-expression/reader.h"
#include "type/bytecode.h"
#include "type/closure.h"
#include "type/symbol.h"

#include <string.h>

extern u8 stdlib_no_data[];
bool lVerbose = false;


/* Initialize the allocator and symbol table, needs to be called before as
 * soon as possible, since most procedures depend on it.*/
void lInit(){
	lArrayInit();
	lClosureInit();
	lNativeFunctionsInit();
	lStringInit();
	lValInit();
	lSymbolInit();
	lTreeInit();
}

/* Evaluate the Nujel Lambda expression and return the results */
lVal *lLambda(lClosure *c, lVal *args, lVal *lambda){
	const int SP = lRootsGet();
	lVal *vn = args;
	lClosure *tmpc = RCP(lClosureNew(lambda->vClosure, closureCall));
	tmpc->text = lambda->vClosure->text;
	tmpc->name = lambda->vClosure->name;
	tmpc->caller = c;
	for(lVal *n = lambda->vClosure->args; n; n = n->vList.cdr){
		if(n->type == ltPair){
			lVal *car = lCar(n);
			if(car){lDefineClosureSym(tmpc, lGetSymbol(car), lCar(vn));}
			vn = lCdr(vn);
		}else if(n->type == ltSymbol){
			if(n){lDefineClosureSym(tmpc, lGetSymbol(n), vn);}
		}else{
			lExceptionThrowValClo("invalid-lambda", "Incorrect type in argument list", lambda, c);
		}
	}
	lVal *ret = lBytecodeEval(tmpc, NULL, &lambda->vClosure->text->vBytecodeArr, false);
	lRootsRet(SP);
	return ret;
}

/* Run fun with args, evaluating args if necessary  */
lVal *lApply(lClosure *c, lVal *args, lVal *fun, lVal *funSym){
	switch(fun ? fun->type : ltNoAlloc){
	case ltMacro:
		lExceptionThrowValClo("runtime-macro","Can't use macros as functions",lCons(funSym,args),c);
	case ltObject: {
		if(args && args->type == ltBytecodeArr){
			RCP(c);
			return lBytecodeEval(fun->vClosure, NULL, &args->vBytecodeArr, false);
		}else{
			lExceptionThrowValClo("no-more-walking", "Can't use the treewalker anymore", args, c);
			return NULL;
		}}
	case ltLambda:
		return lLambda(c,args,fun);
	case ltSpecialForm:
	case ltNativeFunc:
		return fun->vNFunc->fp(c,args);
	default:
		lExceptionThrowValClo("type-error", "Can't apply to following val", fun, c);
		return NULL;
	}
}

static inline bool lArgsEval(const lVal *v){
	return v && ((v->type == ltLambda) || (v->type == ltNativeFunc));
}

/* Directly Evaluate a single value, v, and return the result.
 * DOES NOT COMPILE THE EXPRESSION. */
lVal *lEval(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	default:
		return v;
	case ltSymbol:
		return lGetClosureSym(c,v->vSymbol);
	case ltPair: {
		lVal *car = lCar(v);
		switch(car ? car->type : ltNoAlloc){
		default:
			lExceptionThrowValClo("type-error", "Can't use the following type as a function", lValSymS(getTypeSymbol(car)), c);
                case ltMacro:
			lExceptionThrowValClo("runtime-macro", "Macros can't be used as functions", v, c);
		case ltObject:
			lExceptionThrowValClo("object-car", "Can't apply to objects in that way, use apply or eval-in instead", v, c);
		case ltLambda:
			return RVP(lLambda(c,lMap(c,lCdr(v),lEval),car));
		case ltSpecialForm:
			return RVP(car->vNFunc->fp(c,lCdr(v)));
		case ltNativeFunc:
			return RVP(car->vNFunc->fp(c,lMap(c,lCdr(v),lEval)));
                case ltPair:
			return lApply(c,lMap(c,lCdr(v),lEval),lRootsValPush(lEval(c,car)),car);
		case ltKeyword:
			return v; // ToDo: can probably be removed
		case ltSymbol: {
			lVal *resolved;
			if(lHasClosureSym(c,car->vSymbol,&resolved)){
				lVal *args = lArgsEval(resolved) ? lMap(c,lCdr(v),lEval) : lCdr(v);
				return lApply(c, args, resolved, car);
			}else{
				lExceptionThrowValClo("unresolved-procedure", "Can't resolve the following symbol into a procedure", car, c);
			}}
		}
	}}
	return NULL;
}

/* Evaluate func for every entry in list v and return a list containing the results */
lVal *lMap(lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *)){
	lVal *ret, *cc;
	ret = cc = RVP(lCons(NULL,NULL));
	ret->vList.car = func(c,lCar(v));
	for(lVal *t = lCdr(v); t ; t = lCdr(t)){
		cc = cc->vList.cdr = lCons(NULL,NULL);
		cc->vList.car = func(c,lCar(t));
	}
	return ret;
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

/* Add all the core native functions to c, without IO or stdlib */
static void lAddCoreFuncs(lClosure *c){
	lOperationsAllocation(c);
	lOperationsArithmetic(c);
	lOperationsMath(c);
	lOperationsArray(c);
	lOperationsBinary(c);
	lOperationsBytecode(c);
	lOperationsCore(c);
	lOperationsReader(c);
	lOperationsSpecial(c);
	lOperationsString(c);
	lOperationsTree(c);
	lOperationsTypeSystem(c);
	lOperationsVector(c);
}

/* Create a new root closure WITHOUT loading the nujel stdlib, mostly of interest when testing a different stdlib than the one included */
static lClosure *lNewRootNoStdLib(){
	lClosure *c = lClosureAlloc();
	c->parent = NULL;
	c->type = closureLet;
	lRootsClosurePush(c);
	lAddCoreFuncs(c);
	lAddPlatformVars(c);
	return c;
}

/* Create a new root closure with the default included stdlib */
static void *lNewRootReal(void *a, void *b){
	(void)a; (void)b;
	return lLoad(lNewRootNoStdLib(), (const char *)stdlib_no_data);
}

/* Create a new root closure with the default stdlib using the
 * fallback exception handler */
lClosure *lNewRoot(){
	return lExceptionTryExit(lNewRootReal,NULL,NULL);
}

/* Trigger a break exception as soon as possible, which will most
 * likely abort the current computation and return to the top-level */
void lBreak(){
	breakQueued = true;
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
		}else{
			lBytecodeEval(c, NULL, &car->vBytecodeArr, false);
		}
		lRootsRet(RSSP);
	}
	lRootsRet(RSP);
	return c;
}
