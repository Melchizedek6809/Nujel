/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "casting.h"

#include "nujel.h"
#include "vec.h"
#include "datatypes/closure.h"
#include "datatypes/native-function.h"
#include "datatypes/string.h"
#include "datatypes/vec.h"

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
	lVal *t = lEval(c,v);
	if(t == NULL){return lValInt(0);}
	switch(t->type){
	default: return lValInt(0);
	case ltBool:
		return lValInt(t->vBool ? 1 : 0);
	case ltInt:
		return t;
	case ltFloat:
		return lValInt(t->vFloat);
	case ltVec:
		return lValInt(lVecV(t->vCdr).x);
	case ltString:
		if(t->vCdr == 0){return lValInt(0);}
		return lValInt(atoi(lStrData(t)));
	case ltPair:
		return lnfInt(c,lCar(v));
	}
}

lVal *lnfFloat(lClosure *c, lVal *v){
	if(v == NULL){return lValFloat(0);}
	lVal *t = lEval(c,v);
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default: return lValFloat(0);
	case ltFloat:
		return t;
	case ltInt:
		return lValFloat(t->vInt);
	case ltVec:
		return lValFloat(lVecV(t->vCdr).x);
	case ltString:
		if(t->vCdr == 0){return lValFloat(0);}
		return lValFloat(atof(lStrData(t)));
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
	lVal *a = lEval(c,v);
	if(a == NULL)            {return lValBool(false);}
	if(a->type == ltSymbol)  {a = lResolveSym(c - lClosureList,a);}
	if(a->type == ltBool)    {return a;}
	if(a->type == ltPair)    {a = lCar(a);}
	if(a == NULL)            {return lValBool(false);}
	if(a->type == ltSymbol)  {a = lResolveSym(c - lClosureList,a);}
	if(a->type == ltBool)    {return a;}
	if(a->type == ltPair)    {a = lEval(c,a);}
	if(a == NULL)            {return lValBool(false);}
	if(a->type == ltSymbol)  {a = lResolveSym(c - lClosureList,a);}
	if(a->type == ltBool)    {return a;}
	if(a->type == ltInt)     {return lValBool(a->vInt != 0);}
	return lValBool(true);
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
	lAddNativeFunc(c,"string str","[val]","VAL -> string",lnfCat);
}
