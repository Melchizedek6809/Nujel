/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "casting.h"

#include "nujel.h"
#include "vec.h"
#include "datatypes/closure.h"
#include "datatypes/list.h"
#include "datatypes/native-function.h"
#include "datatypes/string.h"
#include "datatypes/val.h"
#include "datatypes/vec.h"
#include "operations/string.h"

#ifndef COSMOPOLITAN_H_
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
#endif

lVal *lnfInf(lClosure *c, lVal *v){
	(void)c;
	(void)v;
	return lValInf();
}

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
		return lValInt(v->vVec->v.x);
	case ltString:
		if(v->vString == NULL){return lValInt(0);}
		return lValInt(atoi(v->vString->data));
	case ltPair:
		return lnfInt(c,lCar(v));
	}
}

lVal *lnfFloat(lClosure *c, lVal *v){
	if(v == NULL){return lValFloat(0);}
	switch(v->type){
	default: return lValFloat(0);
	case ltFloat:
		return v;
	case ltInt:
		return lValFloat(v->vInt);
	case ltVec:
		return lValFloat(v->vVec->v.x);
	case ltString:
		if(v->vString == NULL){return lValFloat(0);}
		return lValFloat(atof(v->vString->data));
	case ltPair:
		return lnfFloat(c,v->vList.car);
	}
}

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

lVal *lnfBool(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lCar(v) != NULL);
}

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

int castToInt(const lVal *v, int fallback){
	if(v == NULL){return fallback;}
	switch(v->type){
	case ltVec:
		return v->vVec->v.x;
	case ltFloat:
		return v->vFloat;
	case ltInt:
		return v->vInt;
	default:
		return fallback;
	}
}

float castToFloat(const lVal *v, float fallback){
	if(v == NULL){return fallback;}
	switch(v->type){
	case ltVec:
		return v->vVec->v.x;
	case ltFloat:
		return v->vFloat;
	case ltInt:
		return v->vInt;
	default:
		return fallback;
	}
}

vec castToVec(const lVal *v, vec fallback){
	if(v == NULL){return fallback;}
	switch(v->type){
	case ltVec:
		return v->vVec->v;
	case ltFloat:
		return vecNew(v->vFloat,v->vFloat,v->vFloat);
	case ltInt:
		return vecNew(v->vInt,v->vInt,v->vInt);
	default:
		return fallback;
	}
}

bool castToBool(const lVal *v){
	if(v == NULL){return false;}
	if(v->type == ltBool){
		return v->vBool;
	}else if(v->type == ltPair){
		return (v->vList.car != NULL) || (v->vList.cdr != NULL);
	}
	return true;
}

const char *castToString(const lVal *v, const char *fallback){
	if(v == NULL){return fallback;}
	if(v->type != ltString){return fallback;}
	return v->vString->data;
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
	const lVal *car = lCar(a);
	if(car == NULL){return ltNoAlloc;}
	lType ret = car->type;
	forEach(t,lCdr(a)){
		const lVal *tcar = lCar(t);
		ret = lTypecast(ret,tcar == NULL ? ltNoAlloc : tcar->type);
	}
	return ret;
}

void lOperationsCasting(lClosure *c){
	lAddNativeFunc(c,"bool",      "[val]","VAL -> bool ", lnfBool);
	lAddNativeFunc(c,"int",       "[val]","VAL -> int",   lnfInt);
	lAddNativeFunc(c,"float",     "[val]","VAL -> float", lnfFloat);
	lAddNativeFunc(c,"vec",       "[val]","VAL -> vec",   lnfVec);
	lAddNativeFunc(c,"string","[val]","VAL -> string",lnfCat);
}
