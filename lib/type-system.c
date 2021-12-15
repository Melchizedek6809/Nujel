/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
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
#include "collection/list.h"
#include "collection/string.h"
#include "misc/vec.h"
#include "type/closure.h"
#include "type/native-function.h"
#include "type/symbol.h"
#include "type/val.h"
#include "operation.h"

#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


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
	case ltFloat: {
		int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%f",t->vFloat);
		len += clen;
		buf += clen;
		break; }
	case ltInt: {
		int clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf), "%"PRId64 ,t->vInt);
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
	case ltBool:
		return lMap(c,v,lnfBool);
	case ltNoAlloc:
		return NULL;
	}
}

/* Cast v to be an int without memory allocations, or return fallback */
i64 castToInt(const lVal *v, i64 fallback){
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
double castToFloat(const lVal *v, double fallback){
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
	if(v == NULL){
		return false;
	}else if(v->type == ltBool){
		return v->vBool;
	}else if(v->type == ltPair){
		return (v->vList.car != NULL) || (v->vList.cdr != NULL);
	}else{
		return true;
	}
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

/* Return the tree in V if possible, otherwise fallback. */
const lSymbol *castToSymbol(const lVal *v, const lSymbol *fallback){
	if((v == NULL) || (v->type != ltSymbol)){return fallback;}
	return v->vSymbol;
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
	return lValSymS(getTypeSymbol(lCar(v)));
}

/* Add typing and casting operators to c */
void lOperationsTypeSystem(lClosure *c){
	lAddNativeFunc(c,"bool",    "[α]",     "Convert α into a boolean value, true or false", lnfBool);
	lAddNativeFunc(c,"int",     "[α]",     "Convert α into an integer number", lnfInt);
	lAddNativeFunc(c,"float",   "[α]",     "Convert α into a floating-point number", lnfFloat);
	lAddNativeFunc(c,"vec",     "[x y z]", "Convert α into a vector value consistig of 3 floats x,y and z", lnfVec);
	lAddNativeFunc(c,"string",  "[α]",     "Convert α into a printable and readable string", lnfCat);
	lAddNativeFunc(c,"type-of", "[α]",     "Return a symbol describing the type of α", lnfTypeOf);
}
