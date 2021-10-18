/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "nujel.h"

#include "allocator/garbage-collection.h"
#include "misc/random-number-generator.h"
#include "s-expression/reader.h"
#include "s-expression/writer.h"
#include "type-system.h"
#include "types/array.h"
#include "types/closure.h"
#include "types/list.h"
#include "types/native-function.h"
#include "types/string.h"
#include "types/symbol.h"
#include "types/val.h"
#include "types/vec.h"
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
#include "operator/vec.h"

#ifndef COSMOPOLITAN_H_
	#include <ctype.h>
	#include <math.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

extern u8 stdlib_nuj_data[];

char dispWriteBuf[1<<18];

/* Initialize the allocator and symbol table, needs to be called before any other call.*/
void lInit(){
	lInitArray();
	lInitClosure();
	lInitNativeFunctions();
	lInitStr();
	lInitVal();
	lInitVec();
	lInitSymbol();
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
	lClosure *cl = lClosureNew(c);
	if(cl == NULL){return NULL;}
	if((v == NULL) || (lCar(v) == NULL) || (lCdr(v) == NULL)){return NULL;}
	cl->doc  = lCons(lCar(v),lCadr(v));
	cl->text = lWrap(lCdr(v));
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltLambda;
	ret->vClosure = cl;

	forEach(n,lCar(v)){
		lVal *car = lCar(n);
		if((car == NULL) || (car->type != ltSymbol)){continue;}
		lVal *t = lDefineClosureSym(cl,lGetSymbol(car));
		t->vList.car = NULL;
		(void)t;
	}

	return ret;
}

/* Handler for [λ* [..args] docstring body] */
static lVal *lnfLambdaRaw(lClosure *c, lVal *v){
	lClosure *cl = lClosureNew(c);
	if(cl == NULL){return NULL;}
	lVal *doc = lCons(lCar(v),lCadr(v));
	cl->doc  = doc;
	cl->text = lCaddr(v);
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltLambda;
	ret->vClosure = cl;

	forEach(n,lCar(v)){
		lVal *car = lCar(n);
		if((car == NULL) || (car->type != ltSymbol)){continue;}
		lVal *t = lDefineClosureSym(cl,lGetSymbol(car));
		t->vList.car = NULL;
		(void)t;
	}

	return ret;
}

/* Handler for [δ [...args] ...body] */
static lVal *lnfDynamic(lClosure *c, lVal *v){
	lVal *ret = lnfLambda(c,v);
	if(ret == NULL){return NULL;}
	ret->vClosure->flags |= lfDynamic;
	return ret;
}

/* Handler for [ω ...body] */
static lVal *lnfObject(lClosure *c, lVal *v){
	lClosure *cl = lClosureNew(c);
	if(cl == NULL){return NULL;}
	lVal *ret = lValAlloc();
	ret->type = ltLambda;
	ret->vClosure = cl;
	cl->flags |= lfObject;
	lnfDo(cl,v);

	return ret;
}

/* Handler for [self] */
static lVal *lnfSelf(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(c->flags & lfObject){
		lVal *t = lValAlloc();
		t->type = ltLambda;
		t->vClosure = c;
		return t;
	}
	if(c->parent == 0){return NULL;}
	return lnfSelf(c->parent,v);
}

/* Handler for [memory-info] */
static lVal *lnfMemInfo(lClosure *c, lVal *v){
	(void)c; (void)v;
	lVal *ret = NULL;
	ret = lCons(lValInt(lSymbolMax),ret);
	ret = lCons(lValSym(":symbol"),ret);
	ret = lCons(lValInt(lNFuncActive),ret);
	ret = lCons(lValSym(":native-function"),ret);
	ret = lCons(lValInt(lStringActive),ret);
	ret = lCons(lValSym(":string"),ret);
	ret = lCons(lValInt(lClosureActive),ret);
	ret = lCons(lValSym(":array"),ret);
	ret = lCons(lValInt(lArrayActive),ret);
	ret = lCons(lValSym(":vector"),ret);
	ret = lCons(lValInt(lVecActive),ret);
	ret = lCons(lValSym(":closure"),ret);
	ret = lCons(lValInt(lValActive),ret);
	ret = lCons(lValSym(":value"),ret);
	return ret;
}

/* Evaluate the Nujel Lambda expression and return the results */
static lVal *lLambda(lClosure *c,lVal *args, lClosure *lambda){
	if(lambda == NULL){
		lPrintError("lLambda: NULL\n");
		return NULL;
	}
	if(lambda->flags & lfObject){
		return lnfDo(lambda,args);
	}
	lVal *vn = args;
	lClosure *tmpc = 0;
	if(lambda->flags & lfDynamic){
		tmpc = lClosureNew(c);
	}else{
		tmpc = lClosureNew(lambda);
	}
	if(tmpc == NULL){return NULL;}
	tmpc->text = lambda->text;
	forEach(n,lambda->data){
		if(vn == NULL){break;}
		lVal *car = lCaar(n);
		if((car == NULL) || (car->type != ltSymbol)){continue;}
		lSymbol *csym = lGetSymbol(car);
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

	lVal *ret = lEval(tmpc,lambda->text);
	if(tmpc->refCount == 0){
		lClosureFree(tmpc);
	}
	return ret;
}

/* Evaluate a single value, v, and return the result */
lVal *lEval(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}

	if(v->type == ltSymbol){
		return lResolveSym(c,v);
	}else if(v->type == ltPair){
		lVal *ret = lEval(c,lCar(v));
		if(ret == NULL){return v;}
		if(ret->type == ltSpecialForm){
			return ret->vNFunc->fp(c,lCdr(v));
		}else if(ret->type == ltLambda){
			return lLambda(c,lCdr(v),ret->vClosure);
		}
		lVal *args = lMap(c,lCdr(v),lEval);
		switch(ret->type){
		default:
			return v;
		case ltNativeFunc:
			return ret->vNFunc->fp(c,args);
		case ltPair:
			return lEval(c,ret);
		case ltString:
			return lnfCat(c,lCons(ret,args));
		case ltInt:
		case ltFloat:
		case ltVec:
			if(v->vList.cdr == NULL){
				return ret;
			}else{
				return lnfInfix(c,lCons(ret,args));
			}
		case ltArray:
			if(v->vList.cdr == NULL){
				return ret;
			}else{
				return lnfArrRef(c,lCons(ret,args));
			}
		}
	}
	return v;
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
	case ltLambda: {
		lVal *t = lCadr(v);
		if((t == NULL) || (t->type != ltPair)){t = lCons(t,NULL);}
		return lLambda(c,t,func->vClosure);}
	default:
		return v;
	}
}

/* Add all the platform specific constants to c */
static void lAddPlatformVars(lClosure *c){
	#if defined(__HAIKU__)
	lDefineVal(c, "OS", lConst(lValString("Haiku")));
	#elif defined(__APPLE__)
	lDefineVal(c, "OS", lConst(lValString("MacOS")));
	#elif defined(__EMSCRIPTEN__)
	lDefineVal(c, "OS", lConst(lValString("Emscripten")));
	#elif defined(__MINGW32__)
	lDefineVal(c, "OS", lConst(lValString("Windows")));
	#elif defined(__linux__)
	lDefineVal(c, "OS", lConst(lValString("Linux")));
	#else
	lDefineVal(c, "OS", lConst(lValString("*nix")));
	#endif

	#if defined(__arm__)
	lDefineVal(c, "ARCH", lConst(lValString("armv7l")));
	#elif defined(__aarch64__)
	lDefineVal(c, "ARCH", lConst(lValString("aarch64")));
	#elif defined(__x86_64__)
	lDefineVal(c, "ARCH", lConst(lValString("x86_64")));
	#elif defined(__EMSCRIPTEN__)
	lDefineVal(c, "ARCH", lConst(lValString("wasm")));
	#else
	lDefineVal(c, "ARCH", lConst(lValString("unknown")));
	#endif
}

/* Handler for [eval s-expr] */
static lVal *lnfEval(lClosure *c, lVal *v){
	return lEval(c,lCar(v));
}

/* Add all the core native functions to c, without IO or stdlib */
static void lAddCoreFuncs(lClosure *c){
	lOperationsArithmetic(c);
	lOperationsArray(c);
	lOperationsBinary(c);
	lOperationsTypeSystem(c);
	lOperationsClosure(c);
	lOperationsSpecial(c);
	lOperationsList(c);
	lOperationsPredicate(c);
	lOperationsRandom(c);
	lOperationsReader(c);
	lOperationsString(c);
	lOperationsTime(c);
	lOperationsVector(c);

	lAddNativeFunc(c,"apply",           "[func list]",    "Evaluate FUNC with LIST as arguments",       lnfApply);
	lAddNativeFunc(c,"eval",            "[expr]",         "Evaluate EXPR",                              lnfEval);
	lAddNativeFunc(c,"memory-info",     "[]",             "Return memory usage data",                   lnfMemInfo);
	lAddNativeFunc(c,"self",            "[]",             "Return the closest object closure",          lnfSelf);

	lAddSpecialForm(c,"λ*",             "[args source body]", "Create a new, raw, lambda",             lnfLambdaRaw);
	lAddSpecialForm(c,"lambda lam λ \\","[args ...body]", "Create a new lambda",                       lnfLambda);
	lAddSpecialForm(c,"dynamic dyn δ",  "[args ...body]", "New Dynamic scoped lambda",                 lnfDynamic);
	lAddSpecialForm(c,"object obj ω",   "[args ...body]", "Create a new object",                       lnfObject);
}

/* Create a new root closure WITHTOUT loading the nujel stdlib, mostly of interest when testing a different stdlib than the one included */
lClosure *lClosureNewRootNoStdLib(){
	lClosure *c = lClosureAlloc();
	if(c == NULL){return NULL;}
	c->parent = 0;
	c->flags |= lfNoGC;
	lAddCoreFuncs(c);
	lAddPlatformVars(c);
	return c;
}

/* Create a new root closure with the default included stdlib */
lClosure *lClosureNewRoot(){
	lClosure *c = lClosureNewRootNoStdLib();
	lEval(c,lWrap(lRead((const char *)stdlib_nuj_data)));
	return c;
}

/* Evaluate func for every entry in list v and return a list containing the results */
lVal *lMap(lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *)){
	if((c == NULL) || (v == NULL)){return NULL;}
	lVal *ret = NULL, *cc = NULL;

	forEach(t,v){
		lVal *ct = func(c,lCar(t));
		if(ct == NULL){continue;}
		ct = lCons(ct,NULL);
		if(ret == NULL){ret = ct;}
		if(cc  != NULL){cc->vList.cdr = ct;}
		cc = ct;
	}

	return ret;
}

/* Append a do to the beginning of v, useful when evaluating user input via a repl, since otherwise we could only accept a single expression. */
lVal *lWrap(lVal *v){
	return lCons(lValSymS(symDo),v);
}
