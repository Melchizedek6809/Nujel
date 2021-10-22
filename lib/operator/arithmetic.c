/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "arithmetic.h"
#include "../allocation/roots.h"
#include "../nujel.h"
#include "../type-system.h"
#include "../misc/vec.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/val.h"
#include "../type/vec.h"

#ifndef COSMOPOLITAN_H_
	#include <math.h>
	#include <stdlib.h>
	#include <stdio.h>
#endif

static lVal *lnfAddV(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vVec->v = vecAdd(t->vVec->v,lCar(vv)->vVec->v);
	}
	return t;
}
static lVal *lnfAddF(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vFloat += lCar(vv)->vFloat;
	}
	return t;
}
static lVal *lnfAddI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vInt += lCar(vv)->vInt;
	}
	return t;
}
lVal *lnfAdd(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	lCastApply(lnfAdd,c,v);
}


static lVal *lnfSubV(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vVec->v = vecSub(t->vVec->v,lCar(vv)->vVec->v);
	}
	return t;
}
static lVal *lnfSubF(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vFloat -= lCar(vv)->vFloat;
	}
	return t;
}
static lVal *lnfSubI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vInt -= lCar(vv)->vInt;
	}
	return t;
}
lVal *lnfSub(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(0);}
	if((v->type == ltPair) && (lCar(v) != NULL) && (lCdr(v) == NULL)){
		v = lCons(lValInt(0),v);
	}
	lCastApply(lnfSub,c,v);
}

static lVal *lnfMulV(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vVec->v = vecMul(t->vVec->v,lCar(vv)->vVec->v);
	}
	return t;
}
static lVal *lnfMulF(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vFloat *= lCar(vv)->vFloat; }
	return t;
}
static lVal *lnfMulI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){ t->vInt *= lCar(vv)->vInt; }
	return t;
}
lVal *lnfMul(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(1);}
	lCastApply(lnfMul, c , v);
}


static lVal *lnfDivV(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vVec->v = vecDiv(t->vVec->v,lCar(vv)->vVec->v);
	}
	return t;
}
static lVal *lnfDivF(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		const float cv = lCar(vv)->vFloat;
		if(cv == 0){return lValInf();}
		t->vFloat /= cv;
	}
	return t;
}
static lVal *lnfDivI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		if(lCar(vv)->vInt == 0){return lValInf();}
		t->vInt /= lCar(vv)->vInt;
	}
	return t;
}
lVal *lnfDiv(lClosure *c, lVal *v){
	if(v == NULL){return lValInt(1);}
	lCastApply(lnfDiv, c, v);
}



static lVal *lnfModV(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		t->vVec->v = vecMod(t->vVec->v,lCar(vv)->vVec->v);
	}
	return t;
}
static lVal *lnfModF(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		const float cv = lCar(vv)->vFloat;
		if(cv == 0){return lValInf();}
		t->vFloat = fmodf(t->vFloat,cv);
	}
	return t;
}
static lVal *lnfModI(lVal *t, lVal *v){
	forEach(vv,lCdr(v)){
		const int cv = lCar(vv)->vInt;
		if(cv == 0){return lValInf();}
		t->vInt %= cv;
	}
	return t;
}
lVal *lnfMod(lClosure *c, lVal *v){
	lCastApply(lnfMod, c, v);
}

lVal *lnfAbs(lClosure *c, lVal *v){
	lVal *t = lCar(lCastNumeric(c,v));
	if(t == NULL){return lValInt(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(fabsf(t->vFloat));
	case ltInt:
		return lValInt(abs(t->vInt));
	case ltVec:
		return lValVec(vecAbs(t->vVec->v));
	}
}

lVal *lnfSqrt(lClosure *c, lVal *v){
	lVal *t = lCar(lCastNumeric(c,v));
	if(t == NULL){return lValInt(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(sqrtf(t->vFloat));
	case ltInt:
		return lValFloat(sqrtf(t->vInt));
	case ltVec:
		return lValVec(vecSqrt(t->vVec->v));
	}
}

lVal *lnfCeil(lClosure *c, lVal *v){
	lVal *t = lCar(lCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(ceilf(t->vFloat));
	case ltInt:
		return t;
	case ltVec:
		return lValVec(vecCeil(t->vVec->v));
	}
}

lVal *lnfFloor(lClosure *c, lVal *v){
	lVal *t = lCar(lCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(floorf(t->vFloat));
	case ltInt:
		return t;
	case ltVec:
		return lValVec(vecFloor(t->vVec->v));
	}
}

lVal *lnfRound(lClosure *c, lVal *v){
	lVal *t = lCar(lCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(roundf(t->vFloat));
	case ltInt:
		return t;
	case ltVec:
		return lValVec(vecRound(t->vVec->v));
	}
}

lVal *lnfSin(lClosure *c, lVal *v){
	lVal *t = lCar(lCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return lValFloat(sinf(t->vFloat));
	}
}

lVal *lnfCos(lClosure *c, lVal *v){
	lVal *t = lCar(lCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return lValFloat(cosf(t->vFloat));
	}
}

lVal *lnfTan(lClosure *c, lVal *v){
	lVal *t = lCar(lCastNumeric(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
	default:
		return lValFloat(0);
	case ltFloat:
		return lValFloat(tanf(t->vFloat));
	}
}


lVal *lnfPow(lClosure *c, lVal *v){
	if(lCdr(v) == NULL){return lValInt(0);}
	v = lCastNumeric(c,v);
	if(lCdr(v) == NULL){return lValInt(0);}
	lVal *t = lCar(v);
	if(t == NULL){return lValInt(0);}
	lVal *u = lCadr(v);
	if(u == NULL){return lValInt(0);}
	switch(t->type){
	default:
		return lValInt(0);
	case ltFloat:
		return lValFloat(powf(t->vFloat,u->vFloat));
	case ltInt:
		return lValFloat(powf(t->vInt,u->vInt));
	case ltVec:
		return lValVec(vecPow(t->vVec->v,u->vVec->v));
	}
}

lVal *lnfVMag(lClosure *c, lVal *v){
	lVal *t = lCar(lCastSpecific(c,v,ltVec));
	if((t == NULL) || (t->type != ltVec)){return lValFloat(0);}
	return lValFloat(vecMag(t->vVec->v));
}

lVal *infixFunctions[32];
int infixFunctionCount = 0;

void lAddInfix(lVal *v){
	infixFunctions[infixFunctionCount++] = v;
}

lVal *lnfInfix (lClosure *c, lVal *v){
	lVal *l = NULL, *start = NULL;
	if(v == NULL){return NULL;}
	start = l = lRootsValPush(lCons(NULL,NULL));
	start->vList.car = lEval(c,lCar(v));
	for(lVal *cur=lCdr(v);cur != NULL;cur=lCdr(cur)){
		l->vList.cdr = lCons(NULL,NULL);
		l = l->vList.cdr;
		l->vList.car = lEval(c,lCar(cur));
	}
	for(int i=0;i<infixFunctionCount;i++){
		lVal *func;
		for(lVal *cur=start;cur != NULL;cur=lCdr(cur)){
			tryAgain: func = lCadr(cur);
			if(func == NULL){break;}
			if(func->vNFunc != infixFunctions[i]->vNFunc){continue;}
			if(func->type != infixFunctions[i]->type){continue;}
			lVal *args = cur;
			lVal *tmp = args->vList.car;
			args->vList.car = lCadr(args);
			lCdr(args)->vList.car = tmp;
			tmp = lCddr(args)->vList.cdr;
			lCddr(args)->vList.cdr = NULL;
			args->vList.car = lEval(c,args);
			args->vList.cdr = tmp;
			goto tryAgain;
		}
	}
	lRootsValPop();
	return lCar(start);
}

void lOperationsArithmetic(lClosure *c){
	lAddInfix(lAddNativeFunc(c,"mod %",  "[...args]","Modulo",        lnfMod));
	lAddInfix(lAddNativeFunc(c,"div /",  "[...args]","Division",      lnfDiv));
	lAddInfix(lAddNativeFunc(c,"mul *",  "[...args]","Multiplication",lnfMul));
	lAddInfix(lAddNativeFunc(c,"sub -",  "[...args]","Substraction",  lnfSub));
	lAddInfix(lAddNativeFunc(c,"add +",  "[...args]","Addition",      lnfAdd));
	lAddInfix(lAddNativeFunc(c,"pow",    "[a b]",    "Return a raised to the power of b",lnfPow));

	lAddNativeFunc(c,"abs","[a]",  "Return the absolute value of a",   lnfAbs);
	lAddNativeFunc(c,"sqrt","[a]", "Return the squareroot of a",       lnfSqrt);
	lAddNativeFunc(c,"floor","[a]","Round a down",                     lnfFloor);
	lAddNativeFunc(c,"ceil","[a]", "Round a up",                       lnfCeil);
	lAddNativeFunc(c,"round","[a]","Round a",                          lnfRound);
	lAddNativeFunc(c,"sin","[a]",  "Sin A",                            lnfSin);
	lAddNativeFunc(c,"cos","[a]",  "Cos A",                            lnfCos);
	lAddNativeFunc(c,"tan","[a]",  "Tan A",                            lnfTan);

	lAddNativeFunc(c,"vec/length vec/magnitude","[vec]","Return the length of VEC",lnfVMag);
}
