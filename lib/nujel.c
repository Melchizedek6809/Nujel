/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "nujel.h"

#include "allocation/garbage-collection.h"
#include "allocation/roots.h"
#include "collection/array.h"
#include "collection/closure.h"
#include "collection/list.h"
#include "collection/string.h"
#include "collection/tree.h"
#include "misc/random-number-generator.h"
#include "s-expression/reader.h"
#include "s-expression/writer.h"
#include "type-system.h"
#include "type/native-function.h"
#include "type/symbol.h"
#include "type/val.h"
#include "operator/arithmetic.h"
#include "operator/array.h"
#include "operator/binary.h"
#include "operator/closure.h"
#include "operator/special.h"
#include "operator/list.h"
#include "operator/predicates.h"
#include "operator/random.h"
#include "operator/string.h"
#include "operator/time.h"
#include "operator/tree.h"
#include "operator/vec.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern u8 stdlib_no_data[];

char dispWriteBuf[1<<18];
bool lVerbose = false;

/* Initialize the allocator and symbol table, needs to be called before as
 * soon as possible, since most procedures depend on it.*/
void lInit(){
	lInitArray();
	lInitClosure();
	lInitNativeFunctions();
	lInitStr();
	lInitVal();
	lInitSymbol();
	lTreeInit();
}

/* Display v on the default channel, most likely stdout */
void lDisplayVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,true);
	printf("%s",dispWriteBuf);
}

/* Display v on the error channel, most likely stderr */
void lDisplayErrorVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,true);
	fprintf(stderr,"%s",dispWriteBuf);
}

/* Write a machine-readable presentation of v to stdout */
void lWriteVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,false);
	printf("%s\n",dispWriteBuf);
}

/* Write a machine-readable presentation of t to stdout */
void lWriteTree(lTree *t){
	lSWriteTree(t, dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,false);
	printf("%s\n",dispWriteBuf);
}

/* Handler for [λ [...args] ...body] */
static lVal *lnfLambda(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	lVal *cdr = lCdr(v);
	if((v == NULL) || (car == NULL) || (cdr == NULL)){
		return NULL;
	}
	lVal *ret = lRootsValPush(lValAlloc());
	ret->type           = ltLambda;
	ret->vClosure       = lClosureNew(c);
	ret->vClosure->doc  = lCons(car,lCar(cdr));
	ret->vClosure->text = lWrap(cdr);
	ret->vClosure->args = car;

	return ret;
}

/* Handler for [λ* [..args] docstring body] */
static lVal *lnfLambdaRaw(lClosure *c, lVal *v){
	lVal *ret = lRootsValPush(lValAlloc());
	ret->type           = ltLambda;
	ret->vClosure       = lClosureNew(c);
	ret->vClosure->doc  = lCons(lCar(v),lCadr(v));
	ret->vClosure->text = lCaddr(v);
	ret->vClosure->args = lCar(v);

	return ret;
}

/* Handler for [δ [...args] ...body] */
static lVal *lnfDynamic(lClosure *c, lVal *v){
	lVal *ret = lnfLambda(c,v);
	if(ret == NULL){return NULL;}
	ret->type = ltDynamic;
	return ret;
}

/* Handler for [ω ...body] */
static lVal *lnfObject(lClosure *c, lVal *v){
	lVal *ret = lRootsValPush(lValAlloc());
	ret->type     = ltObject;
	ret->vClosure = lClosureNew(c);
	ret->vClosure->type = closureObject;
	lnfDo(ret->vClosure,v);
	return ret;
}

/* Handler for [memory-info] */
static lVal *lnfMemInfo(lClosure *c, lVal *v){
	(void)c; (void)v;
	lVal *ret = lRootsValPush(lCons(NULL,NULL));
	lVal *l = ret;

	l->vList.car = lValSym(":value");
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(lValActive);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;

	l->vList.car = lValSym(":closure");
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(lClosureActive);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;

	l->vList.car = lValSym(":array");
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(lArrayActive);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;

	l->vList.car = lValSym(":string");
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(lStringActive);
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;

	l->vList.car = lValSym(":symbol");
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;
	l->vList.car = lValInt(lSymbolMax);

	return ret;
}

/* Evaluate the Nujel Lambda expression and return the results */
static lVal *lLambda(lClosure *c,lVal *args, lVal *lambda){
	if(lambda == NULL){
		lPrintError("lLambda: NULL\n");
		return NULL;
	}
	if(lambda->type == ltObject){
		return lnfDo(lambda->vClosure,args);
	}
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
		if((car == NULL) || (car->type != ltSymbol)){continue;}
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
static lVal *lApply(lClosure *c, lVal *args, lVal *fun){
	switch(fun ? fun->type : ltNoAlloc){
	case ltObject:
	case ltLambda:
	case ltDynamic:
		return lLambda(c,args,fun);
	case ltSpecialForm:
		return fun->vNFunc->fp(c,args);
	case ltNativeFunc: {
		lVal *evaledArgs = lMap(c,args,lEval);
		lVal *ret = fun->vNFunc->fp(c,evaledArgs);
		return ret;}
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
		if(car == NULL){return NULL;}
		switch(car->type){
		default:
			return v;
		case ltLambda:
		case ltDynamic:
		case ltNativeFunc:
		case ltSpecialForm:
		case ltObject:
			return lApply(c,lCdr(v),car);
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
		case ltSymbol:
			return lSymKeyword(car->vSymbol)
				? v
				: lEval(c,lRootsValPush(lCons(lGetClosureSym(c,car->vSymbol),lCdr(v))));
		case ltPair:
			return lEval(c,lRootsValPush(lCons(lRootsValPush(lEval(c,car)),lCdr(v))));
		}}
	}
}

/* Evaluate func for every entry in list v and return a list containing the results */
lVal *lMap(lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *)){
	if((c == NULL) || (v == NULL)){return NULL;}
	lVal *ret=NULL, *cc=NULL;
	for(lVal *t = v; t ; t = lCdr(t)){
		if(cc == NULL){
			ret = cc = lRootsValPush(lCons(NULL,NULL));
		}else{
			cc->vList.cdr = lCons(NULL,NULL);
			cc = cc->vList.cdr;
		}
		cc->vList.car = func(c,lCar(t));
	}
	return ret;
}

/* Handler for [apply fn list] */
static lVal *lnfApply(lClosure *c, lVal *v){
	lVal *func = lCar(v);
	if(func == NULL){return NULL;}
	if(func->type == ltSymbol){func = lResolveSym(c,func);}
	if(func == NULL){return NULL;}
	switch(func->type){
	case ltSpecialForm:
		return func->vNFunc->fp(c,lCadr(v));
	case ltNativeFunc:
		return func->vNFunc->fp(c,lCadr(v));
	case ltObject: {
		lVal *t = lCadr(v);
		return lLambda(c,t,func);}
	case ltDynamic:
	case ltLambda: {
		lVal *t = lCadr(v);
		return lLambda(c,t,func);}
	default:
		return NULL;
	}
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

/* [eval* expr] - Evaluate the already compiled EXPR */
static lVal *lnfEvalRaw(lClosure *c, lVal *v){
	return lEval(c,lCar(v));
}

/* Add all the core native functions to c, without IO or stdlib */
static void lAddCoreFuncs(lClosure *c){
	lOperationsArithmetic(c);
	lOperationsPredicate(c);
	lOperationsArray(c);
	lOperationsBinary(c);
	lOperationsTypeSystem(c);
	lOperationsClosure(c);
	lOperationsSpecial(c);
	lOperationsList(c);
	lOperationsRandom(c);
	lOperationsReader(c);
	lOperationsString(c);
	lOperationsTime(c);
	lOperationsTree(c);
	lOperationsVector(c);

	lAddNativeFunc(c,"apply",           "[func list]",    "Evaluate FUNC with LIST as arguments",       lnfApply);
	lAddNativeFunc(c,"eval*",           "[expr]",         "Evaluate the already compiled EXPR",         lnfEvalRaw);
	lAddNativeFunc(c,"memory-info",     "[]",             "Return memory usage data",                   lnfMemInfo);

	lAddSpecialForm(c,"λ*",             "[args source body]", "Create a new, raw, lambda",             lnfLambdaRaw);
	lAddSpecialForm(c,"lambda fun λ \\","[args ...body]", "Create a new lambda",                       lnfLambda);
	lAddSpecialForm(c,"dynamic dyn δ",  "[args ...body]", "New Dynamic scoped lambda",                 lnfDynamic);
	lAddSpecialForm(c,"object ω",   "[args ...body]", "Create a new object",                       lnfObject);
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
lClosure *lClosureNewRoot(){
	lClosure *c = lClosureNewRootNoStdLib();
	c->text = lRead((const char *)stdlib_no_data);
	c->text = lWrap(c->text);
	lEval(c,c->text);
	c->text = NULL;
	return c;
}

/* Append a do to the beginning of v, useful when evaluating user input via a repl, since otherwise we could only accept a single expression. */
lVal *lWrap(lVal *v){
	lVal *r = lRootsValPush(lCons(NULL,NULL));
	r->vList.cdr = v;
	r->vList.car = lValSymS(symDo);
	return r;
}
