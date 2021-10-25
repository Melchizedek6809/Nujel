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
#include "type/vec.h"
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

#ifndef COSMOPOLITAN_H_
	#include <ctype.h>
	#include <math.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

extern u8 stdlib_no_data[];

char dispWriteBuf[1<<18];
bool lVerbose = false;

/* Initialize the allocator and symbol table, needs to be called before any other call.*/
void lInit(){
	lInitArray();
	lInitClosure();
	lInitNativeFunctions();
	lInitStr();
	lInitVal();
	lInitVec();
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

/* Handler for [λ [...args] ...body] */
static lVal *lnfLambda(lClosure *c, lVal *v){
	if((v == NULL) || (lCar(v) == NULL) || (lCdr(v) == NULL)){
		return NULL;
	}
	lVal *ret = lRootsValPush(lValAlloc());
	ret->type           = ltLambda;
	ret->vClosure       = lClosureNew(c);
	ret->vClosure->doc  = lCons(lCar(v),lCadr(v));
	ret->vClosure->text = lWrap(lCdr(v));

	forEach(n,lCar(v)){
		lVal *car = lCar(n);
		if((car == NULL) || (car->type != ltSymbol)){continue;}
		lVal *t = lDefineClosureSym(ret->vClosure,lGetSymbol(car));
		t->vList.car = NULL;
		(void)t;
	}

	return lRootsValPop();
}

/* Handler for [λ* [..args] docstring body] */
static lVal *lnfLambdaRaw(lClosure *c, lVal *v){
	lVal *ret = lValAlloc();
	lRootsValPush(ret);
	ret->type           = ltLambda;
	ret->vClosure       = lClosureNew(c);
	ret->vClosure->doc  = lCons(lCar(v),lCadr(v));
	ret->vClosure->text = lCaddr(v);

	forEach(n,lCar(v)){
		lVal *car = lCar(n);
		if((car == NULL) || (car->type != ltSymbol)){continue;}
		lVal *t = lDefineClosureSym(ret->vClosure,lGetSymbol(car));
		t->vList.car = NULL;
		(void)t;
	}
	return lRootsValPop();
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
	//lWriteVal(ret);
	lRootsValPop();
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
	l->vList.cdr = lCons(NULL,NULL);
	l = l->vList.cdr;

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
	lVal *vn = args;
	lClosure *tmpc = (lambda->type == ltDynamic
		? lClosureNew(c)
		: lClosureNew(lambda->vClosure));
	lRootsClosurePush(tmpc);
	tmpc->text = lambda->vClosure->text;
	forEach(n,lambda->vClosure->data){
		if(vn == NULL){break;}
		lVal *car = lCaar(n);
		if((car == NULL) || (car->type != ltSymbol)){continue;}
		const lSymbol *csym = lGetSymbol(car);
		lVal *lv = lDefineClosureSym(tmpc,csym);
		if(lSymVariadic(csym)){
			lVal *t = lSymNoEval(csym) ? vn : lMap(c,vn,lEval);
			if((lv != NULL) && (lv->type == ltPair)){ lv->vList.car = t;}
			break;
		}else{
			lVal *t = lSymNoEval(csym) ? lCar(vn) : lEval(c,lCar(vn));
			if((lv != NULL) && (lv->type == ltPair)){ lv->vList.car = t;}
			if(vn != NULL){vn = lCdr(vn);}
		}
	}

	lVal *ret = lEval(tmpc,lambda->vClosure->text);
	lRootsClosurePop();
	return ret;
}

/* Evaluate a single value, v, and return the result */
lVal *lEval(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}

	lRootsValPush(v);
	lRootsClosurePush(c);
	if(v->type == ltSymbol){
		v = lResolveSym(c,v);
	}else if(v->type == ltPair){
		lVal *ret = lRootsValPush(lEval(c,lCar(v)));
		if(ret == NULL){
			v = NULL;
		}else if(ret->type == ltSpecialForm){
			v = ret->vNFunc->fp(c,lCdr(v));
		}else if((ret->type == ltLambda) || (ret->type == ltDynamic) || (ret->type == ltObject)){
			v = lLambda(c,lCdr(v),ret);
		}else{
			lVal *args = lMap(c,lCdr(v),lEval);
			lRootsValPush(args);
			if(ret->type == ltNativeFunc){
				v = ret->vNFunc->fp(c,args);
			}else{
				lVal *nv = lCons(ret,args);
				lRootsValPush(nv);
				switch(ret->type){
				case ltPair:
					v = lEval(c,nv);
					break;
				case ltString:
					v = lnfCat(c,nv);
					break;
				case ltInt:
				case ltFloat:
				case ltVec:
					v = (v->vList.cdr == NULL) ? ret : lnfInfix(c,nv);
					break;
				case ltArray:
					v = (v->vList.cdr == NULL) ? ret : lnfArrRef(c,nv);
					break;
				case ltTree:
					v = (v->vList.cdr == NULL) ? ret : lnfTreeGet(c,nv);
					break;
				}
				lRootsValPop();
			}
			lRootsValPop();
		}
		lRootsValPop();
	}
	lRootsClosurePop();
	lRootsValPop();
	return v;
}

/* Evaluate func for every entry in list v and return a list containing the results */
lVal *lMap(lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *)){
	if((c == NULL) || (v == NULL)){return NULL;}
	lVal *car = func(c,lCar(v));
	lRootsValPush(car);
	lVal *cc = lCons(car,NULL);
	lRootsValPop();
	lRootsValPush(cc);
	forEach(t,lCdr(v)){
		cc->vList.cdr = lCons(NULL,NULL);
		cc = cc->vList.cdr;
		cc->vList.car = func(c,lCar(t));
	}
	return lRootsValPop();
}

/* Handler for [apply fn list] */
static lVal *lnfApply(lClosure *c, lVal *v){
	lVal *func = lCar(v);
	if(func == NULL){return NULL;}
	if(func->type == ltSymbol){func = lResolveSym(c,func);}
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
		return v;
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
	return lRootsValPop();
}
