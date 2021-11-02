/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 *
 * In this file you will find different subroutines for casting from one type to
 * another, as well as code for determining which type would be most fitting when
 * you have to for example add two values together.
 */
#include "type-system.h"

#include "api.h"
#include "nujel.h"
#include "allocation/val.h"
#include "collection/closure.h"
#include "collection/list.h"
#include "collection/string.h"
#include "misc/vec.h"
#include "type/native-function.h"
#include "type/symbol.h"
#include "type/val.h"
#include "operation/string.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* [inf] - Return infinity */
lVal *lnfInf(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValInf();
}

/* [int v] - Convert v into an integer number */
lVal *lnfInt(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	switch(v->type){
	default: return lValInt(0);
	case ltBool:
		return lValInt(v->vBool ? 1 : 0);
	case ltInt:
		return v;
	case ltFloat:
		return lValInt(v->vFloat);
	case ltVec:
		return lValInt(v->vVec.x);
	case ltString:
		if(v->vString == NULL){return lValInt(0);}
		return lValInt(atoi(v->vString->data));
	case ltPair:
		return lnfInt(c,lCar(v));
	}
}

/* [float v] - Convert v into a floating-point number */
lVal *lnfFloat(lClosure *c, lVal *v){
	if(v == NULL){return lValFloat(0);}
	switch(v->type){
	default: return lValFloat(0);
	case ltFloat:
		return v;
	case ltInt:
		return lValFloat(v->vInt);
	case ltVec:
		return lValFloat(v->vVec.x);
	case ltString:
		if(v->vString == NULL){return lValFloat(0);}
		return lValFloat(atof(v->vString->data));
	case ltPair:
		return lnfFloat(c,v->vList.car);
	}
}

/* [vec v] - Convert v into a vector value consistig of 3 floats x,y and z */
lVal *lnfVec(lClosure *c, lVal *v){
	vec nv = vecNew(0,0,0);
	if(v == NULL){return lValVec(nv);}
	if(v->type == ltVec){return v;}
	if(v->type != ltPair){
		v = lnfFloat(c,v);
		return lValVec(vecNew(v->vFloat,v->vFloat,v->vFloat));
	}
	int i = 0;
	forEach(cv,v){
		lVal *t = lEval(c,lCar(cv));
		if(t == NULL){break;}
		if(t->type == ltVec){return t;}
		t = lnfFloat(c,t);
		if(t == NULL){break;}
		for(int ii=i;ii<3;ii++){
			nv.v[ii] = t->vFloat;
		}
		if(++i >= 3){break;}
	}
	return lValVec(nv);
}

/* [bool v] - Convert v into a boolean value, true or false */
lVal *lnfBool(lClosure *c, lVal *v){
	(void)c;
	return lValBool(castToBool(lCar(v)));
}

/* [string v] - Convert v into a printable and readable string */
lVal *lnfString(lClosure *c, lVal *t){
	char tmpStringBuf[32];
	char *buf = tmpStringBuf;
	int len = 0;
	if(t == NULL){return lValString("");}
	(void)c;

	switch(t->type){
	default: break;
	case ltInf: {
		int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"#inf");
		len += clen;
		buf += clen;
		break; }
	case ltFloat: {
		int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%f",t->vFloat);
		len += clen;
		buf += clen;
		break; }
	case ltInt: {
		int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%i",t->vInt);
		len += clen;
		buf += clen;
		break; }
	case ltBool: {
		int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%s",t->vBool ? "#t" : "#f");
		len += clen;
		buf += clen;
		break; }
	case ltString:
		return t;
	}

	buf[len] = 0;
	lVal *ret    = lValAlloc();
	ret->type    = ltString;
	ret->vString = lStringNew(tmpStringBuf, len);
	return ret;
}

/* Cast all values in list v to be of type t */
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

/* Cast v to be an int without memory allocations, or return fallback */
int castToInt(const lVal *v, int fallback){
	switch(v ? v->type : ltNoAlloc){
	case ltVec:
		return v->vVec.x;
	case ltFloat:
		return v->vFloat;
	case ltInt:
		return v->vInt;
	default:
		return fallback;
	}
}

/* Cast v to be a float without memory allocations, or return fallback */
float castToFloat(const lVal *v, float fallback){
	switch(v ? v->type : ltNoAlloc){
	case ltVec:
		return v->vVec.x;
	case ltFloat:
		return v->vFloat;
	case ltInt:
		return v->vInt;
	default:
		return fallback;
	}
}

/* Cast v to be a vec without memory allocations, or return fallback */
vec castToVec(const lVal *v, vec fallback){
	switch(v ? v->type : ltNoAlloc){
	case ltVec:
		return v->vVec;
	case ltFloat:
		return vecNew(v->vFloat,v->vFloat,v->vFloat);
	case ltInt:
		return vecNew(v->vInt,v->vInt,v->vInt);
	default:
		return fallback;
	}
}

/* Cast v to be a bool without memory allocations, or return false */
bool castToBool(const lVal *v){
	if(v == NULL){return false;}
	if(v->type == ltBool){
		return v->vBool;
	}else if(v->type == ltPair){
		return (v->vList.car != NULL) || (v->vList.cdr != NULL);
	}
	return true;
}

/* Cast v to be a string without memory allocations, or return fallback */
const char *castToString(const lVal *v, const char *fallback){
	if((v == NULL) || (v->type != ltString)){return fallback;}
	return v->vString->data;
}

/* Return the tree in V if possible, otherwise fallback. */
lTree *castToTree(const lVal *v, lTree *fallback){
	if((v == NULL) || (v->type != ltTree)){return fallback;}
	return v->vTree;
}

/* Determine which type has the highest precedence between a and b */
lType lTypecast(const lType a,const lType b){
	if((a == ltInf)   || (b == ltInf))  {return ltInf;}
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

/* Cast v to a value of type */
lVal *lCastSpecific(lClosure *c, lVal *v, const lType type){
	return lCast(c,v,type);
}

/* Determine the numeric type with the highest precedence in list v */
lVal *lCastNumeric(lClosure *c, lVal *v){
	bool castNeeded = false;
	lType type = lTypecastList(v, &castNeeded);
	if(type == ltString){
		type = ltFloat;
		castNeeded = true;
	}
	lVal *ret = castNeeded ? lCast(c,v,type) : v;
	return ret;
}

/* [type-of v] - Return a symbol describing the type of VAL*/
static lVal *lnfTypeOf(lClosure *c, lVal *v){
	(void)c;
	return lValSymS(getTypeSymbol(lCar(v)));
}

/* Add typing and casting operators to c */
void lOperationsTypeSystem(lClosure *c){
	lAddNativeFunc(c,"inf",     "[v]", "Return infinity", lnfInf);
	lAddNativeFunc(c,"bool",    "[v]", "Convert v into a boolean value, true or false", lnfBool);
	lAddNativeFunc(c,"int",     "[v]", "Convert v into an integer number", lnfInt);
	lAddNativeFunc(c,"float",   "[v]", "Convert v into a floating-point number", lnfFloat);
	lAddNativeFunc(c,"vec",     "[v]", "Convert v into a vector value consistig of 3 floats x,y and z", lnfVec);
	lAddNativeFunc(c,"string",  "[v]", "Convert v into a printable and readable string", lnfCat);
	lAddNativeFunc(c,"type-of", "[v]", "Return a symbol describing the type of VAL", lnfTypeOf);
}
