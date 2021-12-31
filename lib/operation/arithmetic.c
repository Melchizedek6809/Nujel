/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../operation.h"

#include "../allocation/roots.h"
#include "../exception.h"
#include "../nujel.h"
#include "../type-system.h"
#include "../misc/vec.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/val.h"

#include <math.h>
#include <stdlib.h>

static lVal *exceptionThrow(lClosure *c, lVal *v, const char *func){
	(void)func;
	lExceptionThrowValClo(":type-error","Can't calculate with non numeric types, please explicitly convert into a numeric form using [int α],[float β],[vec γ].",v, c);
	return NULL;
}
static lVal *exceptionThrowFloat(lClosure *c, lVal *v, const char *func){
	(void)func;
	lExceptionThrowValClo(":type-error","This function can only be used with floats, you can use [float α] to explicitly convert into a floating point value",v, c);
	return NULL;
}

static vec lnfAddV(const lVal *v){
	vec acc = v->vList.car->vVec;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc = vecAdd(acc,v->vList.car->vVec);
	}
	return acc;
}
static double lnfAddF(const lVal *v){
	double acc = v->vList.car->vFloat;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc += v->vList.car->vFloat;
	}
	return acc;
}
static i64 lnfAddI(const lVal *v){
	i64 acc = v->vList.car->vInt;
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
		default:      return exceptionThrow(c, v,"addition");
		case ltInt:   return lValInt(lnfAddI(t));
		case ltFloat: return lValFloat(lnfAddF(t));
		case ltVec:   return lValVec(lnfAddV(t));
	}
}

static lVal *lnfAddAst(lClosure *c, lVal *v){
	(void)c;
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(a == NULL){return b;}
	if(b == NULL){return a;}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrow(c, v,"addition");
		case ltInt:   return lValInt(castToInt(a,0) + castToInt(b,0));
		case ltFloat: return lValFloat(castToFloat(a,0.f) + castToFloat(b,0.f));
		case ltVec:   return lValVec(vecAdd(castToVec(a,vecZero()), castToVec(b,vecZero())));
	}
}

static lVal *lnfSubAst(lClosure *c, lVal *v){
	(void)c;
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(a == NULL){return b;}
	if(b == NULL){return a;}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrow(c, v,"subtraction");
		case ltInt:   return lValInt(castToInt(a,0) - castToInt(b,0));
		case ltFloat: return lValFloat(castToFloat(a,0.f) - castToFloat(b,0.f));
		case ltVec:   return lValVec(vecSub(castToVec(a,vecZero()), castToVec(b,vecZero())));
	}
}

static lVal *lnfMulAst(lClosure *c, lVal *v){
	(void)c;
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(a == NULL){return b;}
	if(b == NULL){return a;}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrow(c, v,"multiplication");
		case ltInt:   return lValInt(castToInt(a,1) * castToInt(b,1));
		case ltFloat: return lValFloat(castToFloat(a,1.f) * castToFloat(b,1.f));
		case ltVec:   return lValVec(vecMul(castToVec(a,vecZero()), castToVec(b,vecZero())));
	}
}

static lVal *lnfDivAst(lClosure *c, lVal *v){
	(void)c;
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(a == NULL){return b;}
	if(b == NULL){return a;}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default: return exceptionThrow(c, v,"division");
		case ltInt: {
			const int av = castToInt(a,1);
			const int bv = castToInt(b,1);
			if(bv == 0){lExceptionThrowValClo(":division-by-zero","Dividing by zero is probably not what you wanted", NULL, c);}
			return lValInt(av / bv);}
		case ltFloat: return lValFloat(castToFloat(a,1.f) / castToFloat(b,1.f));
		case ltVec:   return lValVec(vecDiv(castToVec(a,vecZero()), castToVec(b,vecZero())));
	}
}

static lVal *lnfModAst(lClosure *c, lVal *v){
	(void)c;
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(a == NULL){return b;}
	if(b == NULL){return a;}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrow(c, v,"module");
		case ltInt: {
			const int av = castToInt(a,1);
			const int bv = castToInt(b,1);
			if(bv == 0){lExceptionThrowValClo(":division-by-zero","Module/Dividing by zero is probably not what you wanted", NULL, c);}
			return lValInt(av % bv);}
		case ltFloat: return lValFloat(fmod(castToFloat(a,0.f),castToFloat(b,0.f)));
		case ltVec:   return lValVec(vecMod(castToVec(a,vecZero()), castToVec(b,vecZero())));
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
static double lnfSubF(lVal *v){
	double acc = v->vList.car->vFloat;
	v = v->vList.cdr;
	if(!v){return -acc;}
	for(; v ; v = v->vList.cdr){
		acc -= v->vList.car->vFloat;
	}
	return acc;
}
static i64 lnfSubI(lVal *v){
	i64 acc = v->vList.car->vInt;
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
		default:      return exceptionThrow(c, v,"substraction");
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
static double lnfMulF(lVal *v){
	double acc;
	for(acc = 1.f; v ; v = v->vList.cdr){
		acc *= v->vList.car->vFloat;
	}
	return acc;
}
static i64 lnfMulI(lVal *v){
	i64 acc;
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
		default:      return exceptionThrow(c, v,"multiplication");
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
static double lnfDivF(lVal *v){
	double acc = v->vList.car->vFloat;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc /= v->vList.car->vFloat;
	}
	return acc;
}



static i64 lnfDivI(lClosure *c, lVal *v){
	i64 acc = v->vList.car->vInt;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		if(v->vList.car->vInt == 0){
			lExceptionThrowValClo(":division-by-zero","Dividing by zero is probably not what you wanted", NULL, c);
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
		default:      return exceptionThrow(c, v,"division");
		case ltInt:   return lValInt(lnfDivI(c,t));
		case ltFloat: return lValFloat(lnfDivF(t));
		case ltVec:   return lValVec(lnfDivV(t));
	}
}


static vec lnfModV(lClosure *c, lVal *v){
	(void)c;
	vec acc = v->vList.car->vVec;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc = vecMod(acc,v->vList.car->vVec);
	}
	return acc;
}
static double lnfModF(lClosure *c, lVal *v){
	(void)c;
	double acc = v->vList.car->vFloat;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		acc = fmodf(acc,v->vList.car->vFloat);
	}
	return acc;
}
static i64 lnfModI(lClosure *c, lVal *v){
	i64 acc = v->vList.car->vInt;
	v = v->vList.cdr;
	for(; v ; v = v->vList.cdr){
		if(v->vList.car->vInt == 0){
			lExceptionThrowValClo(":division-by-zero","Modulo/Dividing by zero is probably not what you wanted", NULL, c);
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
		default:      return exceptionThrow(c, v,"modulo");
		case ltInt:   return lValInt(lnfModI(c,t));
		case ltFloat: return lValFloat(lnfModF(c,t));
		case ltVec:   return lValVec(lnfModV(c,t));
	}
}

lVal *lnfPow(lClosure *c, lVal *v){
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	if(a == NULL){return b;}
	if(b == NULL){return a;}
	lType t = lTypecast(a->type, b->type);
	switch(t){
		default:      return exceptionThrowFloat(c, v,"power");
		case ltInt:   return lValInt(pow(castToInt(a,0),castToInt(b,0)));
		case ltFloat: return lValFloat(pow(castToFloat(a,0.f),castToFloat(b,0.f)));
		case ltVec:   return lValVec(vecPow(castToVec(a,vecZero()),castToVec(b,vecZero())));
	}
}

void lOperationsArithmeticInteger(lClosure *c);
void lOperationsArithmetic(lClosure *c){
	lAddNativeFunc(c,"%", "[...args]", "Modulo",        lnfMod);
	lAddNativeFunc(c,"/", "[...args]", "Division",      lnfDiv);
	lAddNativeFunc(c,"*", "[...args]", "Multiplication",lnfMul);
	lAddNativeFunc(c,"-", "[...args]", "Substraction",  lnfSub);
	lAddNativeFunc(c,"+", "[...args]", "Addition",      lnfAdd);
	lAddNativeFunc(c,"** pow", "[a b]", "Return A raised to the power of B",lnfPow);

	lAddNativeFunc(c,"add", "[a b]", "Return a + b",  lnfAddAst);
	lAddNativeFunc(c,"sub", "[a b]", "Return a - b",  lnfSubAst);
	lAddNativeFunc(c,"mul", "[a b]", "Return a * b",  lnfMulAst);
	lAddNativeFunc(c,"div", "[a b]", "Return a / b",  lnfDivAst);
	lAddNativeFunc(c,"mod", "[a b]", "Return a % b",  lnfModAst);

	lOperationsArithmeticInteger(c);
}
