/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "nujel.h"

#include "casting.h"
#include "garbage-collection.h"
#include "reader.h"
#include "random-number-generator.h"
#include "datatypes/array.h"
#include "datatypes/closure.h"
#include "datatypes/native-function.h"
#include "datatypes/string.h"
#include "datatypes/symbol.h"
#include "datatypes/vec.h"
#include "operations/arithmetic.h"
#include "operations/array.h"
#include "operations/binary.h"
#include "operations/closure.h"
#include "operations/conditional.h"
#include "operations/predicates.h"
#include "operations/random.h"
#include "operations/string.h"
#include "operations/time.h"
#include "operations/vec.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern u8 stdlib_nuj_data[];

lVal     lValList[VAL_MAX];
uint     lValActive = 0;
uint     lValMax    = 1;
uint     lValFFree  = 0;

char dispWriteBuf[1<<16];

void lInit(){
	lValActive      = 0;
	lValMax         = 1;

	lInitArray();
	lInitClosure();
	lInitNativeFunctions();
	lInitStr();
	lInitVec();
	lInitSymbol();
}

lVal *lValAlloc(){
	lVal *ret;
	if(lValFFree == 0){
		if(lValMax >= VAL_MAX-1){
			exit(1);
			lPrintError("lVal OOM ");
			return NULL;
		}
		ret = &lValList[lValMax++];
	}else{
		ret       = &lValList[lValFFree & VAL_MASK];
		lValFFree = ret->vCdr;
	}
	lValActive++;
	*ret = (lVal){0};
	return ret;
}

void lGUIWidgetFree(lVal *v);
void lValFree(lVal *v){
	if((v == NULL) || (v->type == ltNoAlloc)){return;}
	if(v->type == ltLambda){
		lClo(v->vCdr).refCount--;
	}else if(v->type == ltGUIWidget){
		lGUIWidgetFree(v);
	}
	lValActive--;
	v->type   = ltNoAlloc;
	v->vCdr   = lValFFree;
	lValFFree = v - lValList;
}

lVal *lValCopy(lVal *dst, const lVal *src){
	if((dst == NULL) || (src == NULL)){return NULL;}
	*dst = *src;
	if(dst->type == ltString){
		dst->vCdr = lStringNew(lStrData(src),lStringLength(&lStr(src)));
	}else if(dst->type == ltVec){
		dst->vCdr = lVecAlloc();
		lVecV(dst->vCdr) = lVecV(src->vCdr);
	}else if(dst->type == ltPair){
		dst->vList.car = lValDup(dst->vList.car);
		dst->vList.cdr = lValDup(dst->vList.cdr);
	}
	return dst;
}

lVal *lValInf(){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltInf;
	return ret;
}

lVal *lValInt(int v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltInt;
	ret->vInt = v;
	return ret;
}

lVal *lValFloat(float v){
	lVal *ret   = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type   = ltFloat;
	ret->vFloat = v;
	return ret;
}
lVal *lValBool(bool v){
	lVal *ret = lValAlloc();
	if(ret == NULL){return ret;}
	ret->type = ltBool;
	ret->vBool = v;
	return ret;
}

lVal *lCons(lVal *car, lVal *cdr){
	lVal *v = lValAlloc();
	if(v == NULL){return NULL;}
	v->type = ltPair;
	v->vList.car = car;
	v->vList.cdr = cdr;
	return v;
}

/* TODO: Both seem to write outside of buf if v gets too long */
void lDisplayVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,true);
	printf("%s",dispWriteBuf);
}

void lDisplayErrorVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,true);
	fprintf(stderr,"%s",dispWriteBuf);
}

void lWriteVal(lVal *v){
	lSWriteVal(v,dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],0,false);
	printf("%s\n",dispWriteBuf);
}

lVal *lnfBegin(lClosure *c, lVal *v){
	lVal *ret = NULL;
	forEach(n,v){
		ret = lEval(c,lCar(n));
	}
	return ret;
}

static lVal *lnfLambda(lClosure *c, lVal *v){
	const uint cli = lClosureNew(c - lClosureList);
	if(cli == 0){return NULL;}
	if((v == NULL) || (lCar(v) == NULL) || (lCdr(v) == NULL)){return NULL;}
	lCloText(cli) = lCdr(v);
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltLambda;
	ret->vCdr = cli;

	forEach(n,lCar(v)){
		if(lGetType(lCar(n)) != ltSymbol){continue;}
		lVal *t = lDefineClosureSym(cli,lGetSymbol(lCar(n)));
		t->vList.car = NULL;
		(void)t;
	}

	return ret;
}

static lVal *lnfDynamic(lClosure *c, lVal *v){
	lVal *ret = lnfLambda(c,v);
	if(ret == NULL){return NULL;}
	lClo(ret->vCdr).flags |= lfDynamic;
	return ret;
}

static lVal *lnfObject(lClosure *c, lVal *v){
	const uint cli = lClosureNew(c - lClosureList);
	if(cli == 0){return NULL;}
	lVal *ret = lValAlloc();
	ret->type = ltLambda;
	ret->vCdr = cli;
	lClo(cli).flags |= lfObject;
	lnfBegin(&lClo(cli),v);

	return ret;
}

static lVal *lnfSelf(lClosure *c, lVal *v){
	if(c == NULL){return NULL;}
	if(c->flags & lfObject){
		lVal *t = lValAlloc();
		t->type = ltLambda;
		t->vCdr = c - lClosureList;
		return t;
	}
	if(c->parent == 0){return NULL;}
	return lnfSelf(&lClosureList[c->parent],v);
}

static lVal *lnfQuote(lClosure *c, lVal *v){
	(void)c;
	return lCar(v);
}

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

static lVal *lLambda(lClosure *c,lVal *v, lClosure *lambda){
	if(lambda == NULL){
		lPrintError("lLambda: NULL\n");
		return NULL;
	}
	if(lambda->flags & lfObject){
		return lnfBegin(lambda,v);
	}
	lVal *vn = v;
	uint tmpci = 0;
	if(lambda->flags & lfDynamic){
		tmpci = lClosureNew(lCloI(c));
	}else{
		tmpci = lClosureNew(lCloI(lambda));
	}
	if(tmpci == 0){return NULL;}
	lClosure *tmpc = &lClo(tmpci);
	tmpc->text = lambda->text;
	forEach(n,lambda->data){
		if(vn == NULL){break;}
		lVal *nn = lCar(n);
		if(lGetType(lCar(nn)) != ltSymbol){continue;}
		lSymbol *csym = lGetSymbol(lCar(nn));
		lVal *lv = lDefineClosureSym(tmpci,csym);
		if(lSymVariadic(csym)){
			lVal *t = lSymNoEval(csym) ? vn : lApply(c,vn,lEval);
			if((lv != NULL) && (lv->type == ltPair)){ lv->vList.car = t;}
			break;
		}else{
			lVal *t = lSymNoEval(csym) ? lCar(vn) : lEval(c,lCar(vn));
			if(t  != NULL && t->type == ltSymbol && !lSymNoEval(csym)){t = lEval(c,t);}
			if((lv != NULL) && (lv->type == ltPair)){ lv->vList.car = t;}
			if(vn != NULL){vn = lCdr(vn);}
		}
	}

	lVal *ret = NULL;
	forEach(n,lambda->text){
		ret = lEval(tmpc,lCar(n));
	}
	if(tmpc->refCount == 0){
		lClosureFree(tmpci);
	}
	return ret;
}

static lVal *lnfCar(lClosure *c, lVal *v){
	return lCar(lEval(c,lCar(v)));
}

static lVal *lnfCdr(lClosure *c, lVal *v){
	return lCdr(lEval(c,lCar(v)));
}

static lVal *lnfCons(lClosure *c, lVal *v){
	return lCons(lEval(c,lCar(v)),lEval(c,lCadr(v)));
}
static lVal *lnfSetCar(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	lVal *car = NULL;
	if((v != NULL) && (v->type == ltPair) && (lCdr(v) != NULL)){car = lEval(c,lCadr(v));}
	t->vList.car = car;
	return t;
}
static lVal *lnfSetCdr(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if((t == NULL) || (t->type != ltPair)){return NULL;}
	lVal *cdr = NULL;
	if((v != NULL) && (v->type == ltPair) && (lCdr(v) != NULL)){cdr = lEval(c,lCadr(v));}
	t->vList.cdr = cdr;
	return t;
}

lVal *lEval(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}

	if(v->type == ltSymbol){
		return lResolveSym(c - lClosureList,v);
	}else if(v->type == ltPair){
		lVal *ret = lEval(c,lCar(v));
		if(ret == NULL){return v;}
		switch(ret->type){
		default:
			return v;
		case ltSpecialForm:
			return lNFN(ret->vCdr).fp(c,lCdr(v));
		case ltNativeFunc:
			return lNFN(ret->vCdr).fp(c,lCdr(v));
		case ltLambda:
			return lLambda(c,lCdr(v),&lClo(ret->vCdr));
		case ltPair:
			return lEval(c,ret);
		case ltString:
			return lnfCat(c,v);
		case ltInt:
		case ltFloat:
		case ltVec:
			return v->vList.cdr == NULL ? ret : lnfInfix(c,v);
		case ltArray:
			return v->vList.cdr == NULL ? ret : lnfArrRef(c,v);
		}
	}
	return v;
}

lVal *lnfApply(lClosure *c, lVal *v){
	lVal *func = lEval(c,lCar(v));
	if(func == NULL){return NULL;}
	if(func->type == ltSymbol){func = lResolveSym(c - lClosureList,func);}
	switch(func->type){
	case ltNativeFunc:
		if(lNFN(func->vCdr).fp == NULL){return v;}
		return lNFN(func->vCdr).fp(c,lEval(c,lCadr(v)));
	case ltLambda: {
		lVal *t = lCadr(v);
		if((t == NULL) || (t->type != ltPair)){t = lCons(t,NULL);}
		return lLambda(c,t,&lClo(func->vCdr));}
	default:
		return v;
	}
}

lVal *lnfRead(lClosure *c, lVal *v){
	lVal *t = lEval(c,v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	uint dup = lStringDup(t->vCdr);
	if(dup == 0){return NULL;}
	t = lReadString(&lStringList[dup]);
	if((t != NULL) && (t->type == ltPair) && (lCar(t) != NULL) && (lCdr(t) == NULL)){
		return lCar(t);
	}else{
		return t;
	}
}

static lVal *lnfTypeOf(lClosure *c, lVal *v){
	v = lEval(c,lCar(v));
	if(v == NULL){return lValSym(":nil");}
	switch(v->type){
	case ltNoAlloc:    return lValSym(":no-alloc");
	case ltBool:       return lValSym(":bool");
	case ltPair:       return lValSym(":pair");
	case ltLambda:     return lValSym(":lambda");
	case ltInt:        return lValSym(":int");
	case ltFloat:      return lValSym(":float");
	case ltVec:        return lValSym(":vec");
	case ltString:     return lValSym(":string");
	case ltSymbol:     return lValSym(":symbol");
	case ltNativeFunc: return lValSym(":native-function");
	case ltInf:        return lValSym(":infinity");
	case ltArray:      return lValSym(":array");
	case ltGUIWidget:  return lValSym(":gui-widget");
	}
	return lValSym(":nil");
}

static void lAddPlatformVars(lClosure *c){
	#if defined(__HAIKU__)
	lDefineVal(c, "OS", lConst(lValString("Haiku")));
	#elif defined(__APPLE__)
	lDefineVal(c, "OS", lConst(lValString("Macos")));
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

static lVal *lnfConstant(lClosure *c, lVal *v){
	lVal *t = lEval(c,lCar(v));
	if(t == NULL){return NULL;}
	t->flags |= lfConst;
	return t;
}

static void lAddCoreFuncs(lClosure *c){
	lOperationsArithmetic(c);
	lOperationsArray(c);
	lOperationsBinary(c);
	lOperationsCasting(c);
	lOperationsClosure(c);
	lOperationsConditional(c);
	lOperationsPredicate(c);
	lOperationsRandom(c);
	lOperationsString(c);
	lOperationsTime(c);
	lOperationsVector(c);

	lAddNativeFunc(c,"car",     "[list]",     "Returs the head of LIST",                          lnfCar);
	lAddNativeFunc(c,"cdr",     "[list]",     "Return the rest of LIST",                          lnfCdr);
	lAddNativeFunc(c,"cons",    "[car cdr]",  "Return a new pair of CAR and CDR",                lnfCons);
	lAddNativeFunc(c,"set-car!","[list car]", "Set the CAR of LIST",                           lnfSetCar);
	lAddNativeFunc(c,"set-cdr!","[list cdr]", "Set the CDR of LIST",                           lnfSetCdr);

	lAddNativeFunc(c,"constant const", "[v]",            "Returns V as a constant",                    lnfConstant);
	lAddNativeFunc(c,"apply",          "[func list]",    "Evaluate FUNC with LIST as arguments",       lnfApply);
	lAddNativeFunc(c,"eval",           "[expr]",         "Evaluate EXPR",                              lEval);
	lAddNativeFunc(c,"read",           "[str]",          "Read and Parses STR as an S-Expression",     lnfRead);
	lAddNativeFunc(c,"memory-info",    "[]",             "Return memory usage data",                   lnfMemInfo);
	lAddNativeFunc(c,"lambda lam λ \\","[args ...body]", "Create a new lambda",                        lnfLambda);
	lAddNativeFunc(c,"dynamic dyn δ",  "[args ...body]", "New Dynamic scoped lambda",                  lnfDynamic);
	lAddNativeFunc(c,"object obj ω",   "[args ...body]", "Create a new object",                        lnfObject);
	lAddNativeFunc(c,"self",           "[]",             "Return the closest object closure",          lnfSelf);
	lAddNativeFunc(c,"type-of",        "[val]",          "Return a symbol describing the type of VAL", lnfTypeOf);

	lAddNativeFunc(c,"begin",     "[...body]",     "Evaluate ...body in order and returns the last result",            lnfBegin);
	lAddNativeFunc(c,"quote",     "[v]",           "Return v as is without evaluating",                                lnfQuote);
}

lClosure *lClosureNewRoot(){
	const uint ci = lClosureAlloc();
	if(ci == 0){return NULL;}
	lClosure *c = &lClosureList[ci];
	c->parent = 0;
	c->flags |= lfNoGC;
	lAddCoreFuncs(c);
	lEval(c,lWrap(lRead((const char *)stdlib_nuj_data)));
	lAddPlatformVars(c);
	return c;
}

lVal  *lApply(lClosure *c, lVal *v, lVal *(*func)(lClosure *,lVal *)){
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

lType lTypecast(const lType a,const lType b){
	if((a == ltInf)   || (b == ltInf))  {return ltInf;}
	if((a == ltVec)   || (b == ltVec))  {return ltVec;}
	if((a == ltFloat) || (b == ltFloat)){return ltFloat;}
	if((a == ltInt)   || (b == ltInt))  {return ltInt;}
	if((a == ltBool)  || (b == ltBool)) {return ltBool;}
	if (a == b){ return a;}
	return ltNoAlloc;
}

lType lTypecastList(lVal *a){
	if((a == NULL) || (a->type != ltPair) || (lCar(a) == NULL)){return ltNoAlloc;}
	lType ret = lGetType(lCar(a));
	forEach(t,lCdr(a)){ret = lTypecast(ret,lGetType(lCar(t)));}
	return ret;
}

lType lGetType(lVal *v){
	return v == NULL ? ltNoAlloc : v->type;
}

lVal *lCast(lClosure *c, lVal *v, lType t){
	switch(t){
	default:
		return v;
	case ltString:
		return lApply(c,v,lnfString);
	case ltInt:
		return lApply(c,v,lnfInt);
	case ltFloat:
		return lApply(c,v,lnfFloat);
	case ltVec:
		return lApply(c,v,lnfVec);
	case ltInf:
		return lApply(c,v,lnfInf);
	case ltBool:
		return lApply(c,v,lnfBool);
	case ltNoAlloc:
		return NULL;
	}
}


lVal *getLArgB(lClosure *c, lVal *v, bool *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfBool(c,lEval(c,lCar(v)));
	if(tlv != NULL){
		*res = tlv->vBool;
	}
	return lCdr(v);
}

lVal *getLArgI(lClosure *c, lVal *v, int *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfInt(c,lEval(c,lCar(v)));
	if(tlv != NULL){
		*res = tlv->vInt;
	}
	return lCdr(v);
}

lVal *getLArgF(lClosure *c, lVal *v, float *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfFloat(c,lEval(c,lCar(v)));
	if(tlv != NULL){
		*res = tlv->vFloat;
	}
	return lCdr(v);
}

lVal *getLArgV(lClosure *c, lVal *v, vec *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfVec(c,lEval(c,lCar(v)));
	if(tlv != NULL){
		*res = lVecV(tlv->vCdr);
	}
	return lCdr(v);
}

lVal *getLArgS(lClosure *c, lVal *v,const char **res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfString(c,lEval(c,lCar(v)));
	if(tlv != NULL){
		*res = lStrData(tlv);
	}
	return lCdr(v);
}

lVal *getLArgL(lClosure *c, lVal *v,lVal **res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lEval(c,lCar(v));
	if((tlv != NULL) && ((tlv->type == ltLambda) || (tlv->type == ltNativeFunc))){
		*res = tlv;
	}
	return lCdr(v);
}

lVal *lValDup(const lVal *v){
	return v == NULL ? NULL : lValCopy(lValAlloc(),v);
}

lVal *lWrap(lVal *v){
	return lCons(lValSymS(symBegin),v);
}

lVal *lEvalCast(lClosure *c, lVal *v){
	lVal *t = lApply(c,v,lEval);
	return lCast(c,t,lTypecastList(t));
}

lVal *lEvalCastSpecific(lClosure *c, lVal *v, const lType type){
	return lCast(c,lApply(c,v,lEval),type);
}

lVal *lEvalCastNumeric(lClosure *c, lVal *v){
	lVal *t = lApply(c,v,lEval);
	lType type = lTypecastList(t);
	if(type == ltString){type = ltFloat;}
	return lCast(c,t,type);
}

lVal *lLastCar(lVal *v){
	forEach(a,v){
		if(lCdr(a) == NULL){return lCar(a);}
	}
	return NULL;
}

lVal *lCar(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.car : NULL;
}

lVal *lCdr(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.cdr : NULL;
}

lVal *lCaar(lVal *v){
	return lCar(lCar(v));
}

lVal *lCadr(lVal *v){
	return lCar(lCdr(v));
}

lVal *lCdar(lVal *v){
	return lCdr(lCar(v));
}

lVal *lCddr(lVal *v){
	return lCdr(lCdr(v));
}

lVal *lCadar(lVal *v){
	return lCar(lCdr(lCar(v)));
}

lVal *lCaddr(lVal *v){
	return lCar(lCdr(lCdr(v)));
}

lVal *lCdddr(lVal *v){
	return lCdr(lCdr(lCdr(v)));
}

int lListLength(lVal *v){
	int i = 0;
	for(lVal *n = v;(n != NULL) && (lCar(n) != NULL); n = lCdr(n)){i++;}
	return i;
}

lVal *lConst(lVal *v){
	v->flags |= lfConst;
	return v;
}
