/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "nujel.h"

#include "api.h"
#include "exception.h"
#include "allocation/native-function.h"
#include "allocation/tree.h"
#include "allocation/symbol.h"
#include "operation.h"
#include "type/native-function.h"
#include "type/symbol.h"

#include <stdio.h>
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

/* Evaluate the Nujel Macro and return the results */
lVal *lMacro(lClosure *c,lVal *args, lVal *lambda){
	(void)c;
	if(lambda->type != ltMacro){
		lExceptionThrowValClo(":macro-apply-error","Trying to use macro-apply on anything but a macro is an error, please fix it",lambda, c);
	}
	const int SP = lRootsGet();
	lVal *vn = args;
	lClosure *tmpc = RCP(lClosureNew(lambda->vClosure));
	tmpc->text = lambda->vClosure->text;
	tmpc->name = lambda->vClosure->name;
	tmpc->type = closureCall;
	tmpc->caller = c;
	for(lVal *n = lambda->vClosure->args; n; n = n->vList.cdr){
		if(n->type == ltPair){
			lDefineClosureSym(tmpc, lGetSymbol(lCar(n)), lCar(vn));
			vn = lCdr(vn);
		}else if(n->type == ltSymbol){
			lDefineClosureSym(tmpc, lGetSymbol(n), vn);
		}else{
			lExceptionThrowValClo(":invalid-macro", "Incorrect type in argument list", lambda, c);
		}
	}
	lVal *ret = lEval(tmpc,lambda->vClosure->text);
	lRootsRet(SP);
	return ret;
}

/* Evaluate the Nujel Lambda expression and return the results */
lVal *lLambda(lClosure *c, lVal *args, lVal *lambda){
	const int SP = lRootsGet();
	lVal *vn = args;
	lClosure *tmpc = RCP(lClosureNew(lambda->vClosure));
	tmpc->text = lambda->vClosure->text;
	tmpc->name = lambda->vClosure->name;
	tmpc->type = closureCall;
	tmpc->caller = c;
	for(lVal *n = lambda->vClosure->args; n; n = n->vList.cdr){
		if(n->type == ltPair){
			lDefineClosureSym(tmpc, lGetSymbol(lCar(n)), lCar(vn));
			vn = lCdr(vn);
		}else if(n->type == ltSymbol){
			lDefineClosureSym(tmpc, lGetSymbol(n), vn);
		}else{
			lExceptionThrowValClo(":invalid-lambda", "Incorrect type in argument list", lambda, c);
		}
	}
	lVal *ret = lEval(tmpc,lambda->vClosure->text);
	lRootsRet(SP);
	return ret;
}

/* Run fun with args, evaluating args if necessary  */
lVal *lApply(lClosure *c, lVal *args, lVal *fun, lVal *funSym){
	switch(fun ? fun->type : ltNoAlloc){
	case ltMacro:
		lExceptionThrowValClo(":runtime-macro","Can't use macros as functions",lCons(funSym,args),c);
	case ltObject:
		return lnfDo(fun->vClosure,args);
	case ltLambda:
		return lLambda(c,args,fun);
	case ltSpecialForm:
	case ltNativeFunc:
		return fun->vNFunc->fp(c,args);
	default:
		lExceptionThrowValClo(":type-error", "Can't apply to following val", fun, c);
		return NULL;
	}
}

/* Run an expression after expanding/compiling it */
lVal *lRun(lClosure *c, lVal *v){
	const int SP = lRootsGet();

	lVal *expr = RVP(lList(3,RVP(lValSym("eval-compile")),
	                         RVP(lCons(lnfvQuote,RVP(lCons(v,NULL)))),
	                         RVP(lValObject(c))));
	lVal *ret = lEval(c,expr);

	lRootsRet(SP);
	return ret;
}

/* Read and run an expression after expanding/compiling it */
lVal *lRunS(lClosure *c, const char *s, int sLen){
	const int SP = lRootsGet();

	lVal *expr = RVP(lList(3,RVP(lValSym("read-eval-compile")),
	                         RVP(lValStringConst(s, sLen)),
	                         RVP(lValObject(c))));
	lVal *ret = lEval(c,expr);

	lRootsRet(SP);
	return ret;
}

/* Load an expression expanding/compiling/evaluating it, multiple times if necessary */
void lLoad(lClosure *c, lVal *v){
	const int SP = lRootsGet();
	lEval(c,RVP(lList(3,RVP(lValSym("eval-load")),
	                    RVP(lCons(lnfvQuote,RVP(lCons(v,NULL)))),
	                    RVP(lValObject(c)))));
	lRootsRet(SP);
}

/* Load and compile a string, evaluating parts of it multiple times to enable circular references */
void lLoadS(lClosure *c, const char *s, int sLen){
	const int SP = lRootsGet();
	lEval(c,RVP(lList(3,RVP(lValSym("read-eval-load")),
	                    RVP(lValStringConst(s, sLen)),
	                    RVP(lValObject(c)))));
	lRootsRet(SP);
}

static bool lArgsEval(const lVal *v){
	switch(v ? v->type : ltNoAlloc){
	default:
		return false;
	case ltLambda:
	case ltNativeFunc:
		return true;
	}
}

/* Directly Evaluate a single value, v, and return the result.
 * DOES NOT COMPILE THE EXPRESSION. */
lVal *lEval(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	default:
		return v;
	case ltSymbol:
		return lSymKeyword(v->vSymbol) ? v : lGetClosureSym(c,v->vSymbol);
	case ltPair: {
		lVal *car = lCar(v);
		switch(car ? car->type : ltNoAlloc){
		default:
			lExceptionThrowValClo(":type-error", "Can't use the following type as a function", lValSymS(getTypeSymbol(car)), c);
			return v;
		case ltObject:
			return lnfDo(car->vClosure,lCdr(v));
		case ltLambda:
			return RVP(lLambda(c,lMap(c,lCdr(v),lEval),car));
		case ltMacro:
			lExceptionThrowValClo(":runtime-macro", "Can't use macros as functions", v, c);
			return NULL;
		case ltSpecialForm:
			return RVP(car->vNFunc->fp(c,lCdr(v)));
		case ltNativeFunc:
			return RVP(car->vNFunc->fp(c,lMap(c,lCdr(v),lEval)));
		case ltSymbol: {
			lVal *resolved;
			if(lHasClosureSym(c,car->vSymbol,&resolved)){
				lVal *args = lArgsEval(resolved) ? lMap(c,lCdr(v),lEval) : lCdr(v);
				return lApply(c, args, resolved, car);
			}else{
				if(car->vSymbol && lSymKeyword(car->vSymbol)){
					return v;
				}else{
					lExceptionThrowValClo(":unresolved-procedure", "Can't resolve the following symbol into a procedure", car, c);
					return NULL;
				}
			}}
		case ltPair:
			return lApply(c,lMap(c,lCdr(v),lEval),lRootsValPush(lEval(c,car)),car);
		}}
	}
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
	#if defined(__HAIKU__)
	lDefineVal(c, "OS", lValString("Haiku"));
	#elif defined(__APPLE__)
	lDefineVal(c, "OS", lValString("MacOS"));
	#elif defined(__EMSCRIPTEN__)
	lDefineVal(c, "OS", lValString("Emscripten"));
	#elif defined(__MINGW32__)
	lDefineVal(c, "OS", lValString("Windows"));
	#elif defined(__MSYS__)
	lDefineVal(c, "OS", lValString("Legacy Windows"));
	#elif defined(__MINIX__) || defined(__MINIX3__)
	lDefineVal(c, "OS", lValString("Minix"));
	#elif defined(__linux__)
		#if defined(__UCLIBC__)
		lDefineVal(c, "OS", lValString("uClibc/Linux"));
		#elif defined(__GLIBC__)
		lDefineVal(c, "OS", lValString("GNU/Linux"));
		#elif defined(__BIONIC__)
		lDefineVal(c, "OS", lValString("Android"));
		#else
		lDefineVal(c, "OS", lValString("musl?/Linux"));
		#endif
	#elif defined(__FreeBSD__)
	lDefineVal(c, "OS", lValString("FreeBSD"));
	#elif defined(__OpenBSD__)
	lDefineVal(c, "OS", lValString("OpenBSD"));
	#elif defined(__NetBSD__)
	lDefineVal(c, "OS", lValString("NetBSD"));
	#elif defined(__DragonFly__)
	lDefineVal(c, "OS", lValString("DragonFlyBSD"));
	#else
	lDefineVal(c, "OS", lValString("Unknown"));
	#endif

	#if defined(__arm__)
	lDefineVal(c, "ARCH", lValString("armv7l"));
	#elif defined(__aarch64__)
	lDefineVal(c, "ARCH", lValString("aarch64"));
	#elif defined(__x86_64__) || defined(__amd64__)
	lDefineVal(c, "ARCH", lValString("x86_64"));
	#elif defined(__i386__)
	lDefineVal(c, "ARCH", lValString("x86"));
	#elif defined(__EMSCRIPTEN__)
	lDefineVal(c, "ARCH", lValString("wasm"));
	#else
	lDefineVal(c, "ARCH", lValString("unknown"));
	#endif
}

/* Add all the core native functions to c, without IO or stdlib */
static void lAddCoreFuncs(lClosure *c){
	lOperationsAllocation(c);
	lOperationsArithmetic(c);
	lOperationsMath(c);
	lOperationsArray(c);
	lOperationsBinary(c);
	lOperationsBytecode(c);
	lOperationsClosure(c);
	lOperationsEval(c);
	lOperationsList(c);
	lOperationsPredicate(c);
	lOperationsReader(c);
	lOperationsSpecial(c);
	lOperationsString(c);
	lOperationsTime(c);
	lOperationsTree(c);
	lOperationsTypeSystem(c);
	lOperationsVector(c);
}

/* Evaluate BODYRAW IN C while using CATCHRAW as an exception handler */
lVal *lTry(lClosure *c, lVal *catchRaw, lVal *bodyRaw){
	lVal *volatile catch = catchRaw;
	lVal *volatile body  = bodyRaw;

	const int SP = lRootsGet();
	jmp_buf oldExceptionTarget;
	memcpy(oldExceptionTarget,exceptionTarget,sizeof(jmp_buf));

	lVal *doRet;
	int ret;
	exceptionTargetDepth++;
	ret = setjmp(exceptionTarget);
	if(ret){
		lRootsRet(SP);
		memcpy(exceptionTarget,oldExceptionTarget,sizeof(jmp_buf));
		exceptionTargetDepth--;

		lVal *args = lRootsValPush(exceptionValue);
		args = lRootsValPush(lCons(args,NULL));

		readClosure = NULL;
		doRet = lApply(c,args,catch, NULL);

		return doRet;
	}else{
		doRet = lnfDo(c,body);

		memcpy(exceptionTarget,oldExceptionTarget,sizeof(jmp_buf));
		exceptionTargetDepth--;

		return doRet;
	}
}

/* Quote V, enabling it to be used verbatim, without being evaluated. */
lVal *lQuote(lVal *v){
	lVal *ret = RVP(lCons(NULL,NULL));
	ret->vList.car = lValSymS(symQuote);
	ret->vList.cdr = lCons(v,NULL);
	return ret;
}

/* Create a new root closure WITHTOUT loading the nujel stdlib, mostly of interest when testing a different stdlib than the one included */
lClosure *lNewRootNoStdLib(){
	lClosure *c = lClosureAlloc();
	c->parent = NULL;
	c->type = closureObject;
	lRootsClosurePush(c);
	lAddCoreFuncs(c);
	lAddPlatformVars(c);
	return c;
}

/* Create a new root closure with the default included stdlib */
static void *lNewRootReal(void *a, void *b){
	(void)a; (void)b;
	lClosure *c = lNewRootNoStdLib();
	c->text = lRead((const char *)stdlib_no_data);
	lnfDo(c,c->text);
	c->text = NULL;
	return c;
}

/* Create a new root closure with the default stdlib using the
 * fallback exception handler */
lClosure *lNewRoot(){
	return lExceptionTryExit(lNewRootReal,NULL,NULL);
}

void lBreak(){
	breakQueued = true;
}
