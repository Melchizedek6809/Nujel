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
#include "datatypes/list.h"
#include "datatypes/native-function.h"
#include "datatypes/string.h"
#include "datatypes/symbol.h"
#include "datatypes/val.h"
#include "datatypes/vec.h"
#include "operations/arithmetic.h"
#include "operations/array.h"
#include "operations/binary.h"
#include "operations/closure.h"
#include "operations/special.h"
#include "operations/list.h"
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

char dispWriteBuf[1<<18];

void lInit(){
	lInitArray();
	lInitClosure();
	lInitNativeFunctions();
	lInitStr();
	lInitVal();
	lInitVec();
	lInitSymbol();
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

static lVal *lnfLambda(lClosure *c, lVal *v){
	const uint cli = lClosureNew(c - lClosureList);
	if(cli == 0){return NULL;}
	if((v == NULL) || (lCar(v) == NULL) || (lCdr(v) == NULL)){return NULL;}
	lCloSource(cli) = lCdr(v);
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

static lVal *lnfLambdaRaw(lClosure *c, lVal *v){
	const uint cli = lClosureNew(c - lClosureList);
	if(cli == 0){return NULL;}
	lCloSource(cli) = lCadr(v);
	lCloText(cli) = lCddr(v);
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltLambda;
	ret->vCdr = cli;
	lVal *args = lEval(c,lCar(v));

	forEach(n,args){
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
			lVal *t = lSymNoEval(csym) ? vn : lMap(c,vn,lEval);
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

lVal *lEval(lClosure *c, lVal *v){
	if((c == NULL) || (v == NULL)){return NULL;}

	if(v->type == ltSymbol){
		return lResolveSym(c - lClosureList,v);
	}else if(v->type == ltPair){
		lVal *ret = lEval(c,lCar(v));
		if(ret == NULL){return v;}
		if(ret->type == ltSpecialForm){
			return lNFN(ret->vCdr).fp(c,lCdr(v));
		}else if(ret->type == ltLambda){
			return lLambda(c,lCdr(v),&lClo(ret->vCdr));
		}
		lVal *args = lMap(c,lCdr(v),lEval);
		switch(ret->type){
		default:
			return v;
		case ltNativeFunc:
			return lNFN(ret->vCdr).fp(c,args);
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

lVal *lnfApply(lClosure *c, lVal *v){
	lVal *func = lCar(v);
	if(func == NULL){return NULL;}
	if(func->type == ltSymbol){func = lResolveSym(c - lClosureList,func);}
	switch(func->type){
	case ltSpecialForm:
		if(lNFN(func->vCdr).fp == NULL){return NULL;}
		return lNFN(func->vCdr).fp(c,lCadr(v));
	case ltNativeFunc:
		if(lNFN(func->vCdr).fp == NULL){return NULL;}
		return lNFN(func->vCdr).fp(c,lCadr(v));
	case ltLambda: {
		lVal *t = lCadr(v);
		if((t == NULL) || (t->type != ltPair)){t = lCons(t,NULL);}
		return lLambda(c,t,&lClo(func->vCdr));}
	default:
		return v;
	}
}

lVal *lnfRead(lClosure *c, lVal *v){
	(void)c;
	lVal *t = lCar(v);
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
	(void)c;
	v = lCar(v);
	if(v == NULL){return lValSym(":nil");}
	switch(v->type){
	case ltNoAlloc:    return lValSymS(lSymLTNoAlloc);
	case ltBool:       return lValSymS(lSymLTBool);
	case ltPair:       return lValSymS(lSymLTPair);
	case ltLambda:     return lValSymS(lSymLTLambda);
	case ltInt:        return lValSymS(lSymLTInt);
	case ltFloat:      return lValSymS(lSymLTFloat);
	case ltVec:        return lValSymS(lSymLTVec);
	case ltString:     return lValSymS(lSymLTString);
	case ltSymbol:     return lValSymS(lSymLTSymbol);
	case ltNativeFunc: return lValSymS(lSymLTNativeFunction);
	case ltSpecialForm:return lValSymS(lSymLTSpecialForm);
	case ltInf:        return lValSymS(lSymLTInfinity);
	case ltArray:      return lValSymS(lSymLTArray);
	case ltGUIWidget:  return lValSymS(lSymLTGUIWidget);
	}
	return lValSym(":nil");
}

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

lVal *lConst(lVal *v){
	if(v == NULL){
		return v;
	}
	v->flags |= lfConst;
	return v;
}

static lVal *lnfConstant(lClosure *c, lVal *v){
	(void)c;
	return lConst(lCar(v));
}

static lVal *lnfEval(lClosure *c, lVal *v){
	return lEval(c,lCar(v));
}

static void lAddCoreFuncs(lClosure *c){
	lOperationsArithmetic(c);
	lOperationsArray(c);
	lOperationsBinary(c);
	lOperationsCasting(c);
	lOperationsClosure(c);
	lOperationsSpecial(c);
	lOperationsList(c);
	lOperationsPredicate(c);
	lOperationsRandom(c);
	lOperationsString(c);
	lOperationsTime(c);
	lOperationsVector(c);

	lAddNativeFunc(c,"apply",          "[func list]",    "Evaluate FUNC with LIST as arguments",       lnfApply);
	lAddNativeFunc(c,"eval",           "[expr]",         "Evaluate EXPR",                              lnfEval);
	lAddNativeFunc(c,"read",           "[str]",          "Read and Parses STR as an S-Expression",     lnfRead);
	lAddNativeFunc(c,"memory-info",    "[]",             "Return memory usage data",                   lnfMemInfo);
	lAddNativeFunc(c,"self",           "[]",             "Return the closest object closure",          lnfSelf);
	lAddNativeFunc(c,"type-of",        "[val]",          "Return a symbol describing the type of VAL", lnfTypeOf);
	lAddNativeFunc(c,"constant const", "[v]",            "Returns V as a constant",                    lnfConstant);

	lAddSpecialForm(c,"λ*",             "[args source body]", "Create a new, raw, lambda",             lnfLambdaRaw);
	lAddSpecialForm(c,"lambda lam λ \\","[args ...body]", "Create a new lambda",                       lnfLambda);
	lAddSpecialForm(c,"dynamic dyn δ",  "[args ...body]", "New Dynamic scoped lambda",                 lnfDynamic);
	lAddSpecialForm(c,"object obj ω",   "[args ...body]", "Create a new object",                       lnfObject);
}

lClosure *lClosureNewRootNoStdLib(){
	const uint ci = lClosureAlloc();
	if(ci == 0){return NULL;}
	lClosure *c = &lClosureList[ci];
	c->parent = 0;
	c->flags |= lfNoGC;
	lAddCoreFuncs(c);
	lAddPlatformVars(c);
	return c;
}

lClosure *lClosureNewRoot(){
	lClosure *c = lClosureNewRootNoStdLib();
	lEval(c,lWrap(lRead((const char *)stdlib_nuj_data)));
	return c;
}

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
		return lMap(c,v,lnfString);
	case ltInt:
		return lMap(c,v,lnfInt);
	case ltFloat:
		return lMap(c,v,lnfFloat);
	case ltVec:
		return lMap(c,v,lnfVec);
	case ltInf:
		return lMap(c,v,lnfInf);
	case ltBool:
		return lMap(c,v,lnfBool);
	case ltNoAlloc:
		return NULL;
	}
}


lVal *getLArgB(lClosure *c, lVal *v, bool *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfBool(c,lCar(v));
	if(tlv != NULL){
		*res = tlv->vBool;
	}
	return lCdr(v);
}

lVal *getLArgI(lClosure *c, lVal *v, int *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfInt(c,lCar(v));
	if(tlv != NULL){
		*res = tlv->vInt;
	}
	return lCdr(v);
}

lVal *getLArgF(lClosure *c, lVal *v, float *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfFloat(c,lCar(v));
	if(tlv != NULL){
		*res = tlv->vFloat;
	}
	return lCdr(v);
}

lVal *getLArgV(lClosure *c, lVal *v, vec *res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfVec(c,lCar(v));
	if(tlv != NULL){
		*res = lVecV(tlv->vCdr);
	}
	return lCdr(v);
}

lVal *getLArgS(lClosure *c, lVal *v,const char **res){
	if((v == NULL) || (v->type != ltPair)){return NULL;}
	lVal *tlv = lnfString(c,lCar(v));
	if(tlv != NULL){
		*res = lStrData(tlv);
	}
	return lCdr(v);
}

lVal *lWrap(lVal *v){
	return lCons(lValSymS(symDo),v);
}

lVal *lCastAuto(lClosure *c, lVal *v){
	lVal *t = lMap(c,v,lEval);
	return lCast(c,t,lTypecastList(t));
}

lVal *lCastSpecific(lClosure *c, lVal *v, const lType type){
	return lCast(c,v,type);
}

lVal *lCastNumeric(lClosure *c, lVal *v){
	lType type = lTypecastList(v);
	if(type == ltString){type = ltFloat;}
	return lCast(c,v,type);
}
