/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
/*
 * In this file you will find different subroutines for casting from one type to
 * another, as well as code for determining which type would be most fitting when
 * you have to for example add two values together.
 */
#include "type-system.h"
#include "allocation/symbol.h"
#include "collection/list.h"
#include "collection/string.h"
#include "display.h"
#include "exception.h"
#include "misc/pf.h"
#include "misc/vec.h"
#include "operation.h"
#include "type/closure.h"
#include "type/native-function.h"
#include "type/symbol.h"

#include <stdlib.h>

/* [bool v] - Convert v into a boolean value, true or false */
lVal *lnfBool(lClosure *c, lVal *v){
	(void)c;
	return lValBool(castToBool(lCar(v)));
}

/* Cast it's argument into a string represenation, should only be used by cast/map  */
static lVal *lCastString(lClosure *c, lVal *v){
	(void)c;
	spf(dispWriteBuf,&dispWriteBuf[sizeof(dispWriteBuf)],"%V",v);
	return lValString(dispWriteBuf);
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

/* [int v] - Convert v into an integer number */
lVal *lnfInt(lClosure *c, lVal *v){
	return lCastInt(c, lCar(v));
}

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

/* [float v] - Convert v into a floating-point number */
lVal *lnfFloat(lClosure *c, lVal *v){
	return lCastFloat(c,lCar(v));
}

static lVal *lCastVec(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	case ltVec:
		return v;
	case ltInt:
		return lValVec(vecNew(v->vInt, v->vInt, v->vInt));
	case ltFloat:
		return lValVec(vecNew(v->vFloat, v->vFloat, v->vFloat));
	default:
		lExceptionThrowValClo("type-error", "Can't convert this to a :vec", v, c);
		return NULL;
	}
}

/* Cast all values in list v to be of type t */
lVal *lCast(lClosure *c, lVal *v, lType t){
	switch(t){
	default:
		return v;
	case ltString:
		return lMap(c,v,lCastString);
	case ltInt:
		return lMap(c,v,lCastInt);
	case ltFloat:
		return lMap(c,v,lCastFloat);
	case ltVec:
		return lMap(c,v,lCastVec);
	case ltBool:
		return lMap(c,v,lnfBool);
	case ltNoAlloc:
		return NULL;
	}
}

/* Determine which type has the highest precedence between a and b */
lType lTypecast(const lType a,const lType b){
	if((a == ltVec)   || (b == ltVec))  {return ltVec;}
	if((a == ltFloat) || (b == ltFloat)){return ltFloat;}
	if((a == ltInt)   || (b == ltInt))  {return ltInt;}
	if((a == ltBool)  || (b == ltBool)) {return ltBool;}
	if (a == b){ return a;}
	return ltNoAlloc;
}

/* Determine the type with the highest precedence in the list a */
static lType lTypecastList(lVal *a, bool *castNeeded){
	const lVal *car = lCar(a);
	if(car == NULL){return ltNoAlloc;}
	lType ret = car->type;
	for(lVal *t=a; t ; t = lCdr(t)){
		const lVal *tcar = lCar(t);
		const lType tt = tcar == NULL ? ltNoAlloc : tcar->type;
		if(tt != ret){
			*castNeeded = true;
			ret = lTypecast(ret,tcar == NULL ? ltNoAlloc : tcar->type);
		}
	}
	return ret;
}

/* Cast the list v to their type of highest precedence */
lVal *lCastAuto(lClosure *c, lVal *v){
	lVal *ev = v;
	bool castNeeded = false;
	lType t   = lTypecastList(ev, &castNeeded);
	lVal *ret = castNeeded ? lCast(c,ev,t) : ev;
	return ret;
}

/* [type-of v] - Return a symbol describing the type of VAL*/
static lVal *lnfTypeOf(lClosure *c, lVal *v){
	(void)c;
	return lValKeywordS(getTypeSymbol(lCar(v)));
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

i64 requireInt(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltInt)){
		lExceptionThrowValClo("type-error", "Expected an int, not: ", v, c);
	}
	return v->vInt;
}

double requireFloat(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	case ltFloat:
		return v->vFloat;
	case ltInt:
		return v->vInt;
	}
	lExceptionThrowValClo("type-error", "Expected a float, not: ", v, c);
}

vec requireVec(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltVec)){
		lExceptionThrowValClo("type-error", "Expected a vector, not: ", v, c);
	}
	return v->vVec;
}

/* [vec v] - Convert v into a vector value consisting of 3 floats x,y and z */
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

/* Add typing and casting operators to c */
void lOperationsTypeSystem(lClosure *c){
	lAddNativeFunc(c,"bool",            "[α]",     "Convert α into a boolean value, true or false", lnfBool);
	lAddNativeFunc(c,"int",             "[α]",     "Convert α into an integer number", lnfInt);
	lAddNativeFunc(c,"float",           "[α]",     "Convert α into a floating-point number", lnfFloat);
	lAddNativeFunc(c,"vec",             "[x y z]", "Convert α into a vector value consisting of 3 floats x,y and z", lnfVec);
	lAddNativeFunc(c,"string",          "[α]",     "Convert α into a printable and readable string", lnfCat);
	lAddNativeFunc(c,"symbol->keyword", "[α]",     "Convert symbol α into a keyword", lnfSymbolToKeyword);
	lAddNativeFunc(c,"keyword->symbol", "[α]",     "Convert keyword α into a symbol", lnfKeywordToSymbol);
	lAddNativeFunc(c,"type-of",         "[α]",     "Return a symbol describing the type of α", lnfTypeOf);
}
