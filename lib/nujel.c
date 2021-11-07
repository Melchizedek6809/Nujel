/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "nujel.h"

#include "api.h"
#include "exception.h"
#include "allocation/tree.h"
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
static lVal *lLambda(lClosure *c,lVal *args, lVal *lambda){
	const int SP = lRootsGet();
	lVal *vn = args;
	lClosure *tmpc = (lambda->type == ltDynamic
		? lClosureNew(c)
		: lClosureNew(lambda->vClosure));
	lRootsClosurePush(tmpc);
	tmpc->text = lambda->vClosure->text;
	forEach(n,lambda->vClosure->args){
		if(vn == NULL){break;}
		lVal *car = lCar(n);
		if(car == NULL){continue;}
		const lSymbol *csym = lGetSymbol(car);
		if(lSymVariadic(csym)){
			lVal *t = lSymNoEval(csym) ? vn : lMap(c,vn,lEval);
			lDefineClosureSym(tmpc,csym,t);
			break;
		}else{
			lVal *t = lSymNoEval(csym) ? lCar(vn) : lEval(c,lCar(vn));
			lDefineClosureSym(tmpc,csym,t);
			if(vn != NULL){vn = lCdr(vn);}
		}
	}
	lVal *ret = lEval(tmpc,lambda->vClosure->text);
	lRootsRet(SP);
	return ret;
}

/* Run fun with args, evaluating args if necessary  */
lVal *lApply(lClosure *c, lVal *args, lVal *fun){
	switch(fun ? fun->type : ltNoAlloc){
	case ltObject:
		return lnfDo(fun->vClosure,args);
	case ltLambda:
	case ltDynamic:
		return lLambda(c,args,fun);
	case ltSpecialForm:
		return fun->vNFunc->fp(c,args);
	case ltNativeFunc:
		return fun->vNFunc->fp(c,lMap(c,args,lEval));
	case ltInt:
	case ltFloat:
	case ltVec:
		return lApply(c,lRootsValPush(lCons(fun,args)),lnfvInfix);
	case ltArray:
		return lApply(c,lRootsValPush(lCons(fun,args)),lnfvArrRef);
	case ltString:
		return lApply(c,lRootsValPush(lCons(fun,args)),lnfvCat);
	case ltTree:
		return lApply(c,lRootsValPush(lCons(fun,args)),lnfvTreeGet);
	default:
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
			lExceptionThrowVal(":type-error", "Can't use the following type as a function", lRootsValPush(lValSymS(getTypeSymbol(car))));
			return v;
		case ltObject:
			return lnfDo(car->vClosure,lCdr(v));
		case ltLambda:
		case ltDynamic:
			return lRootsValPush(lLambda(c,lCdr(v),car));
		case ltSpecialForm:
			return car->vNFunc->fp(c,lCdr(v));
		case ltNativeFunc:
			return car->vNFunc->fp(c,lMap(c,lCdr(v),lEval));
		case ltInt:
		case ltFloat:
		case ltVec:
			return lApply(c,v,lnfvInfix);
		case ltArray:
			return lApply(c,v,lnfvArrRef);
		case ltString:
			return lApply(c,v,lnfvCat);
		case ltTree:
			return lApply(c,v,lnfvTreeGet);
		case ltSymbol: {
			lVal *resolved;
			if(lHasClosureSym(c,car->vSymbol,&resolved)){
				return lApply(c,lCdr(v),resolved);
			}else{
				if(car->vSymbol && lSymKeyword(car->vSymbol)){
					return v;
				}
				lExceptionThrowVal(":unresolved-procedure", "Can't resolve the following symbol into a procedure", car);
				return NULL;
			}}
		case ltPair:
			return lApply(c,lCdr(v),lRootsValPush(lEval(c,car)));
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

/* Add all the platform specific constants to c */
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
	lDefineVal(c, "OS", lValString("Linux"));
	#else
	lDefineVal(c, "OS", lValString("*nix"));
	#endif

	#if defined(__arm__)
	lDefineVal(c, "ARCH", lValString("armv7l"));
	#elif defined(__aarch64__)
	lDefineVal(c, "ARCH", lValString("aarch64"));
	#elif defined(__x86_64__)
	lDefineVal(c, "ARCH", lValString("x86_64"));
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

		doRet = lApply(c,args,catch);

		return doRet;
	}else{
		doRet = lnfDo(c,body);

		memcpy(exceptionTarget,oldExceptionTarget,sizeof(jmp_buf));
		exceptionTargetDepth--;

		return doRet;
	}
}

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

static void *lClosureNewRootReal(void *a, void *b){
	(void)a; (void)b;
	lClosure *c = lClosureNewRootNoStdLib();
	c->text = lRead((const char *)stdlib_no_data);
	lnfDo(c,c->text);
	c->text = NULL;
	return c;
}

/* Create a new root closure with the default included stdlib */
lClosure *lClosureNewRoot(){
	return lExceptionTry(lClosureNewRootReal,NULL,NULL);
}
