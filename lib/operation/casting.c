/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
/*
 * In this file you will find different subroutines for casting from one type to
 * another, as well as code for determining which type would be most fitting when
 * you have to for example add two values together.
 */
#include "../operation.h"
#include "../exception.h"
#include "../type-system.h"
#include "../misc/vec.h"
#include "../type/closure.h"
#include "../type/symbol.h"
#include "../type/val.h"

static lVal *lCastFloat(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	case ltFloat:
		return v;
	case ltInt:
		return lValFloat(v->vInt);
	default:
		lExceptionThrowValClo("type-error", "Can't convert this to a :float", v, c);
		return NULL;
	}
}
static lVal *lnfFloat(lClosure *c, lVal *v){
	return lCastFloat(c,lCar(v));
}

static lVal *lCastInt(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	case ltInt:
		return v;
	case ltFloat:
		return lValInt(v->vFloat);
	default:
		lExceptionThrowValClo("type-error", "Can't convert this to a :int", v, c);
		return NULL;
	}
}
static lVal *lnfInt(lClosure *c, lVal *v){
	return lCastInt(c, lCar(v));
}

static lVal *lnfBool(lClosure *c, lVal *v){
	(void)c;
	return lValBool(castToBool(lCar(v)));
}

static lVal *lnfSymbolToKeyword(lClosure *c, lVal *v){
	if((v == NULL) || (v->vList.car == NULL)){
		lExceptionThrowValClo("arity-error","- expects at least 1 argument", v, c);
	}
	if(v->vList.car->type != ltSymbol){
		lExceptionThrowValClo("type-error","expected argument of type :symbol", v->vList.car, c);
	}
	return lValKeywordS(v->vList.car->vSymbol);
}

static lVal *lnfKeywordToSymbol(lClosure *c, lVal *v){
	if((v == NULL) || (v->vList.car == NULL)){
		lExceptionThrowValClo("arity-error","- expects at least 1 argument", v, c);
	}
	if(v->vList.car->type != ltKeyword){
		lExceptionThrowValClo("type-error","expected argument of type :keyword", v->vList.car, c);
	}
	return lValSymS(v->vList.car->vSymbol);
}

lVal *lnfVec(lClosure *c, lVal *v){
	vec nv = vecNew(0,0,0);
	if(v == NULL){return lValVec(nv);}
	if(v->type != ltPair){
		if(v->type == ltInt){
			return lValVec(vecNew(v->vInt, v->vInt, v->vInt));
		}else if(v->type == ltFloat){
			return lValVec(vecNew(v->vFloat, v->vFloat, v->vFloat));
		}else if(v->type == ltVec){
			return v;
		}
	}
	int i = 0;
	for(lVal *cv = v; cv && cv->type == ltPair; cv = cv->vList.cdr){
		lVal *t = lCar(cv);
		if(t == NULL){break;}
		switch(t->type){
		case ltInt:
			nv.v[i] = t->vInt;
			break;
		case ltFloat:
			nv.v[i] = t->vFloat;
			break;
		case ltVec:
			if(i == 0){return t;}
			lExceptionThrowValClo("type-error", "vectors can't contain other vectors, only :float and :int values", t, c);
		default:
			lExceptionThrowValClo("type-error", "Unexpected value in [vec]", t, c);
			break;
		}
		if(++i >= 3){break;}
	}
	for(int ii=MAX(1,i);ii<3;ii++){
		nv.v[ii] = nv.v[ii-1];
	}
	return lValVec(nv);
}

void lOperationsCasting(lClosure *c){
	lAddNativeFunc(c,"bool",            "[α]",     "Convert α into a boolean value, true or false", lnfBool);
	lAddNativeFunc(c,"int",             "[α]",     "Convert α into an integer number", lnfInt);
	lAddNativeFunc(c,"float",           "[α]",     "Convert α into a floating-point number", lnfFloat);
	lAddNativeFunc(c,"vec",             "[x y z]", "Convert α into a vector value consisting of 3 floats x,y and z", lnfVec);
	lAddNativeFunc(c,"string",          "[α]",     "Convert α into a printable and readable string", lnfCat);
	lAddNativeFunc(c,"symbol->keyword", "[α]",     "Convert symbol α into a keyword", lnfSymbolToKeyword);
	lAddNativeFunc(c,"keyword->symbol", "[α]",     "Convert keyword α into a symbol", lnfKeywordToSymbol);
}
