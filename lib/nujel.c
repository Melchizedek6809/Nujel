/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "nujel.h"

#include "api.h"
#include "exception.h"
#include "allocation/native-function.h"
#include "allocation/tree.h"
#include "allocation/symbol.h"
#include "operation/allocation.h"
#include "operation/arithmetic.h"
#include "operation/array.h"
#include "operation/binary.h"
#include "operation/closure.h"
#include "operation/eval.h"
#include "operation/list.h"
#include "operation/predicates.h"
#include "operation/special.h"
#include "operation/string.h"
#include "operation/time.h"
#include "operation/tree.h"
#include "operation/vec.h"
#include "type/native-function.h"
#include "type/symbol.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

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
		lExceptionThrowVal(":macro-apply-error","Trying to use macro-apply on anything but a macro is an error, please fix it",lambda);
	}
	const int SP = lRootsGet();
	lVal *vn = args;
	lClosure *tmpc = lClosureNew(lambda->vClosure);
	lRootsClosurePush(tmpc);
	tmpc->text = lambda->vClosure->text;
	forEach(n,lambda->vClosure->args){
		lVal *car = lCar(n);
		if(car == NULL){continue;}
		const lSymbol *csym = lGetSymbol(car);
		if(vn == NULL){
			lDefineClosureSym(tmpc,csym,NULL);
		}else if(lSymVariadic(csym)){
			lDefineClosureSym(tmpc,csym,vn);
			break;
		}else{
			lDefineClosureSym(tmpc,csym,lCar(vn));
			vn = lCdr(vn);
		}
	}
	lVal *ret = lEval(tmpc,lambda->vClosure->text);
	lRootsRet(SP);
	return ret;
}

/* Evaluate the Nujel Lambda expression and return the results */
lVal *lLambda(lClosure *c,lVal *args, lVal *lambda){
	const int SP = lRootsGet();
	lVal *vn = args;
	lClosure *tmpc = lClosureNew(lambda->vClosure);
	lRootsClosurePush(tmpc);
	tmpc->text = lambda->vClosure->text;
	forEach(n,lambda->vClosure->args){
		lVal *car = lCar(n);
		if(car == NULL){continue;}
		const lSymbol *csym = lGetSymbol(car);
		if(vn == NULL){
			lDefineClosureSym(tmpc,csym,NULL);
		}else if(lSymVariadic(csym)){
			lDefineClosureSym(tmpc,csym,lMap(c,vn,lEval));
			break;
		}else{
			lDefineClosureSym(tmpc,csym,lEval(c,lCar(vn)));
			vn = lCdr(vn);
		}
	}
	lVal *ret = lEval(tmpc,lambda->vClosure->text);
	lRootsRet(SP);
	return ret;
}

/* Run fun with args, evaluating args if necessary  */
lVal *lApply(lClosure *c, lVal *args, lVal *fun, lVal *funSym){
	(void)funSym;
	switch(fun ? fun->type : ltNoAlloc){
	case ltMacro:
		lExceptionThrowVal(":runtime-macro", "Can't use macros as functions", lCons(funSym,args));
	case ltObject:
		return lnfDo(fun->vClosure,args);
	case ltLambda:
		return lLambda(c,args,fun);
	case ltSpecialForm:
		return fun->vNFunc->fp(c,args);
	case ltNativeFunc:
		return fun->vNFunc->fp(c,lMap(c,args,lEval));
	case ltInt:
	case ltFloat:
	case ltVec:
		return lApply(c,lRootsValPush(lCons(fun,args)),lnfvInfix, NULL);
	case ltArray:
		return lApply(c,lRootsValPush(lCons(fun,args)),lnfvArrRef, NULL);
	case ltString:
		return lApply(c,lRootsValPush(lCons(fun,args)),lnfvCat, NULL);
	case ltTree:
		return lApply(c,lRootsValPush(lCons(fun,args)),lnfvTreeGet, NULL);
	default:
		lExceptionThrowVal(":type-error", "Can't apply to following val", fun);
		return NULL;
	}
}

/* Evaluate a single value, v, and return the result */
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
			lExceptionThrowVal(":type-error", "Can't use the following type as a function", lValSymS(getTypeSymbol(car)));
			return v;
		case ltObject:
			return lnfDo(car->vClosure,lCdr(v));
		case ltLambda:
			return lRootsValPush(lLambda(c,lCdr(v),car));
		case ltMacro:
			lExceptionThrowVal(":runtime-macro", "Can't use macros as functions", v);
		case ltSpecialForm:
			return car->vNFunc->fp(c,lCdr(v));
		case ltNativeFunc:
			return car->vNFunc->fp(c,lMap(c,lCdr(v),lEval));
		case ltInt:
		case ltFloat:
		case ltVec:
			return lApply(c,v,lnfvInfix, NULL);
		case ltArray:
			return lApply(c,v,lnfvArrRef, NULL);
		case ltString:
			return lApply(c,v,lnfvCat, NULL);
		case ltTree:
			return lApply(c,v,lnfvTreeGet, NULL);
		case ltSymbol: {
			lVal *resolved;
			if(lHasClosureSym(c,car->vSymbol,&resolved)){
				return lApply(c,lCdr(v),resolved, car);
			}else{
				if(car->vSymbol && lSymKeyword(car->vSymbol)){
					return v;
				}else{
					lExceptionThrowVal(":unresolved-procedure", "Can't resolve the following symbol into a procedure", car);
					return NULL;
				}
			}}
		case ltPair:
			return lApply(c,lCdr(v),lRootsValPush(lEval(c,car)),car);
		}}
	}
}

/* Evaluate func for every entry in list v and return a list containing the results */
lVal *lMap(lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *)){
	if(v == NULL){return NULL;}
	lVal *ret, *cc, *car = lRootsValPush(func(c,lCar(v)));
	ret = cc = lRootsValPush(lCons(car,NULL));
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
	lOperationsArray(c);
	lOperationsBinary(c);
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
	lVal *ret = lRootsValPush(lCons(NULL,NULL));
	ret->vList.car = lValSymS(symQuote);
	ret->vList.cdr = lCons(v,NULL);
	return ret;
}

/* Create a new root closure WITHTOUT loading the nujel stdlib, mostly of interest when testing a different stdlib than the one included */
lClosure *lClosureNewRootNoStdLib(){
	lClosure *c = lClosureAlloc();
	c->parent = NULL;
	lRootsClosurePush(c);
	lAddCoreFuncs(c);
	lAddPlatformVars(c);
	return c;
}

/* Create a new root closure with the default included stdlib */
static void *lClosureNewRootReal(void *a, void *b){
	(void)a; (void)b;
	lClosure *c = lClosureNewRootNoStdLib();
	c->text = lRead((const char *)stdlib_no_data);
	lnfDo(c,c->text);
	c->text = NULL;
	return c;
}

/* Create a new root closure with the default stdlib using the
 * fallback exception handler */
lClosure *lClosureNewRoot(){
	return lExceptionTry(lClosureNewRootReal,NULL,NULL);
}

lVal *lList(int length, ...){
	lVal *ret = NULL, *l;
	va_list varArgs;
	va_start(varArgs,length);
	for(;length;length--){
		lVal *t = va_arg(varArgs, lVal *);
		if(ret == NULL){
			ret = l = RVP(lCons(NULL,NULL));
		}else{
			l = l->vList.cdr = lCons(NULL,NULL);
		}
		l->vList.car = t;
	}
	va_end(varArgs);
	return ret;
}
