/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "arithmetic.h"
#include "../allocation/roots.h"
#include "../display.h"
#include "../exception.h"
#include "../nujel.h"
#include "../type-system.h"
#include "../misc/vec.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/val.h"

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

lVal *lnfvInfix;

static vec lnfAddV(const lVal *v){
	vec acc = v->vList.car->vVec;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc = vecAdd(acc,v->vList.car->vVec);
	}
	return acc;
}
static float lnfAddF(const lVal *v){
	float acc = v->vList.car->vFloat;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc += v->vList.car->vFloat;
	}
	return acc;
}
static int lnfAddI(const lVal *v){
	int acc = v->vList.car->vInt;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc += v->vList.car->vInt;
	}
	return acc;
}
static lVal *lnfAdd(lClosure *c, lVal *v){
	lVal *t = lCastAuto(c,v);
	if((t == NULL) || (t->vList.car == NULL)){return lValInt(0);}
	lRootsValPush(t);
	switch(t->vList.car->type){
		default:      return lValInt(0);
		case ltInf:   return lValInf();
		case ltInt:   return lValInt(lnfAddI(t));
		case ltFloat: return lValFloat(lnfAddF(t));
		case ltVec:   return lValVec(lnfAddV(t));
	}
}


static vec lnfSubV(lVal *v){
	vec acc = v->vList.car->vVec;
	v = v->vList.cdr;
	if(!v){return vecSub(vecZero(),acc);}
	for(; v ; v = v->vList.cdr){
		acc = vecSub(acc,v->vList.car->vVec);
	}
	return acc;
}
static float lnfSubF(lVal *v){
	float acc = v->vList.car->vFloat;
	v = v->vList.cdr;
	if(!v){return -acc;}
	for(; v ; v = v->vList.cdr){
		acc -= v->vList.car->vFloat;
	}
	return acc;
}
static int lnfSubI(lVal *v){
	int acc = v->vList.car->vInt;
	v = v->vList.cdr;
	if(!v){return -acc;}
	for(; v ; v = v->vList.cdr){
		acc -= v->vList.car->vInt;
	}
	return acc;
}
static lVal *lnfSub(lClosure *c, lVal *v){
	lVal *t = lCastAuto(c,v);
	if((t == NULL) || (t->vList.car == NULL)){return lValInt(0);}
	lRootsValPush(t);
	switch(t->vList.car->type){
		default:      return lValInt(0);
		case ltInf:   return lValInf();
		case ltInt:   return lValInt(lnfSubI(t));
		case ltFloat: return lValFloat(lnfSubF(t));
		case ltVec:   return lValVec(lnfSubV(t));
	}
}

static vec lnfMulV(lVal *v){
	vec acc;
	for(acc = vecOne(); v ; v = v->vList.cdr){
		acc = vecMul(acc,v->vList.car->vVec);
	}
	return acc;
}
static float lnfMulF(lVal *v){
	float acc;
	for(acc = 1.f; v ; v = v->vList.cdr){
		acc *= v->vList.car->vFloat;
	}
	return acc;
}
static int lnfMulI(lVal *v){
	int acc;
	for(acc = 1; v ; v = v->vList.cdr){
		acc *= v->vList.car->vInt;
	}
	return acc;
}
lVal *lnfMul(lClosure *c, lVal *v){
	lVal *t = lCastAuto(c,v);
	if((t == NULL) || (t->vList.car == NULL)){return lValInt(0);}
	lRootsValPush(t);
	switch(t->vList.car->type){
		default:      return lValInt(0);
		case ltInf:   return lValInf();
		case ltInt:   return lValInt(lnfMulI(t));
		case ltFloat: return lValFloat(lnfMulF(t));
		case ltVec:   return lValVec(lnfMulV(t));
	}
}


static vec lnfDivV(lVal *v){
	vec acc = v->vList.car->vVec;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc = vecDiv(acc,v->vList.car->vVec);
	}
	return acc;
}
static float lnfDivF(lVal *v){
	float acc = v->vList.car->vFloat;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc /= v->vList.car->vFloat;
	}
	return acc;
}
static int lnfDivI(lVal *v){
	int acc = v->vList.car->vInt;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		if(v->vList.car->vInt == 0){
			lExceptionThrow(":divide-by-zero","Divide by Zero");
			return 0;
		}
		acc /= v->vList.car->vInt;
	}
	return acc;
}
lVal *lnfDiv(lClosure *c, lVal *v){
	lVal *t = lCastAuto(c,v);
	if((t == NULL) || (t->vList.car == NULL)){return lValInt(0);}
	lRootsValPush(t);
	switch(t->vList.car->type){
		default:      return lValInt(0);
		case ltInf:   return lValInf();
		case ltInt:   return lValInt(lnfDivI(t));
		case ltFloat: return lValFloat(lnfDivF(t));
		case ltVec:   return lValVec(lnfDivV(t));
	}
}


static vec lnfModV(lVal *v){
	vec acc = v->vList.car->vVec;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc = vecMod(acc,v->vList.car->vVec);
	}
	return acc;
}
static float lnfModF(lVal *v){
	float acc = v->vList.car->vFloat;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc = fmodf(acc,v->vList.car->vFloat);
	}
	return acc;
}
static int lnfModI(lVal *v){
	int acc = v->vList.car->vInt;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		if(v->vList.car->vInt == 0){
			lPrintError("% 0");
			return 0;
		}
		acc = acc % v->vList.car->vInt;
	}
	return acc;
}
lVal *lnfMod(lClosure *c, lVal *v){
	lVal *t = lCastAuto(c,v);
	if(t == NULL){return lValInt(0);}
	lRootsValPush(t);
	switch(t->vList.car->type){
		default:      return lValInt(0);
		case ltInf:   return lValInf();
		case ltInt:   return lValInt(lnfModI(t));
		case ltFloat: return lValFloat(lnfModF(t));
		case ltVec:   return lValVec(lnfModV(t));
	}
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
		return lValVec(vecAbs(t->vVec));
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
		return lValVec(vecSqrt(t->vVec));
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
		return lValVec(vecCeil(t->vVec));
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
		return lValVec(vecFloor(t->vVec));
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
		return lValVec(vecRound(t->vVec));
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
		return lValVec(vecPow(t->vVec,u->vVec));
	}
}

lVal *lnfVMag(lClosure *c, lVal *v){
	lVal *t = lCar(lCastSpecific(c,v,ltVec));
	if((t == NULL) || (t->type != ltVec)){return lValFloat(0);}
	return lValFloat(vecMag(t->vVec));
}

lVal *infixFunctions[32];
int infixFunctionCount = 0;

void lAddInfix(lVal *v){
	infixFunctions[infixFunctionCount++] = v;
}

lVal *lnfInfix (lClosure *c, lVal *v){
	lVal *l = NULL, *start = NULL;
	if(v == NULL){return NULL;}
	if(v->vList.cdr == NULL){return v->vList.car;}
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
	return lCar(start);
}

void lOperationsArithmetic(lClosure *c){
	lnfvInfix = lAddNativeFunc(c,"infix",  "[...body]","Evaluate body as an infix expression", lnfInfix);
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
