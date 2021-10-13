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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
		return lValInt(lVecV(v->vCdr).x);
	case ltString:
		if(v->vCdr == 0){return lValInt(0);}
		return lValInt(atoi(lStrData(v)));
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
		return lValFloat(lVecV(v->vCdr).x);
	case ltString:
		if(v->vCdr == 0){return lValFloat(0);}
		return lValFloat(atof(lStrData(v)));
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
	return lValBool(lCar(v));
}

bool lBool(const lVal *v){
	if(v == NULL){return false;}
	if(v->type == ltBool){
		return v->vBool;
	}else if(v->type == ltPair){
		return (v->vList.car != NULL) || (v->vList.cdr != NULL);
	}
	return true;
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
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vCdr = lStringNew(tmpStringBuf, len);
	return ret;
}

void lOperationsCasting(lClosure *c){
	lAddNativeFunc(c,"bool",      "[val]","VAL -> bool ", lnfBool);
	lAddNativeFunc(c,"int",       "[val]","VAL -> int",   lnfInt);
	lAddNativeFunc(c,"float",     "[val]","VAL -> float", lnfFloat);
	lAddNativeFunc(c,"vec",       "[val]","VAL -> vec",   lnfVec);
	lAddNativeFunc(c,"string","[val]","VAL -> string",lnfCat);
}
