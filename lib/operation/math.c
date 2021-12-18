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

lVal *lnfAbs(lClosure *c, lVal *v){
	lVal *t = lCar(lCastAuto(c,v));
	if(t == NULL){return lValInt(0);}
	switch(t->type){
		default:      return exceptionThrow(c, v,"absolute");
		case ltFloat: return lValFloat(fabs(t->vFloat));
		case ltInt:   return lValInt(llabs(t->vInt));
		case ltVec:   return lValVec(vecAbs(t->vVec));
	}
}

lVal *lnfCbrt(lClosure *c, lVal *v){
	lVal *t = lCar(lCastAuto(c,v));
	if(t == NULL){return lValInt(0);}
	switch(t->type){
		default:      return exceptionThrow(c, v,"squareroot");
		case ltFloat: return lValFloat(cbrt(t->vFloat));
		case ltInt:   return lValFloat(cbrt(t->vInt));
		case ltVec:   return lValVec(vecCbrt(t->vVec));
	}
}

lVal *lnfSqrt(lClosure *c, lVal *v){
	lVal *t = lCar(lCastAuto(c,v));
	if(t == NULL){return lValInt(0);}
	switch(t->type){
		default:      return exceptionThrow(c, v,"squareroot");
		case ltFloat: return lValFloat(sqrt(t->vFloat));
		case ltInt:   return lValFloat(sqrt(t->vInt));
		case ltVec:   return lValVec(vecSqrt(t->vVec));
	}
}

lVal *lnfCeil(lClosure *c, lVal *v){
	lVal *t = lCar(lCastAuto(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
		default:      return exceptionThrow(c, v,"ceil");
		case ltFloat: return lValFloat(ceil(t->vFloat));
		case ltVec:   return lValVec(vecCeil(t->vVec));
	}
}

lVal *lnfFloor(lClosure *c, lVal *v){
	lVal *t = lCar(lCastAuto(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
		default:      return exceptionThrow(c, v,"floor");
		case ltFloat: return lValFloat(floor(t->vFloat));
		case ltVec:   return lValVec(vecFloor(t->vVec));
	}
}

lVal *lnfRound(lClosure *c, lVal *v){
	lVal *t = lCar(lCastAuto(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
		default:      return exceptionThrow(c, v,"round");
		case ltFloat: return lValFloat(round(t->vFloat));
		case ltVec:   return lValVec(vecRound(t->vVec));
	}
}

lVal *lnfSin(lClosure *c, lVal *v){
	lVal *t = lCar(lCastAuto(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
		default:      return exceptionThrowFloat(c, v,"sinus");
		case ltFloat: return lValFloat(sin(t->vFloat));
	}
}

lVal *lnfCos(lClosure *c, lVal *v){
	lVal *t = lCar(lCastAuto(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
		default:      return exceptionThrowFloat(c, v,"cosine");
		case ltFloat: return lValFloat(cos(t->vFloat));
	}
}

lVal *lnfTan(lClosure *c, lVal *v){
	lVal *t = lCar(lCastAuto(c,v));
	if(t == NULL){return lValFloat(0);}
	switch(t->type){
		default:      return exceptionThrowFloat(c, v,"tangent");
		case ltFloat: return lValFloat(tan(t->vFloat));
	}
}

lVal *lnfVMag(lClosure *c, lVal *v){
	lVal *t = lCar(lCast(c,v,ltVec));
	if((t == NULL) || (t->type != ltVec)){return lValFloat(0);}
	return lValFloat(vecMag(t->vVec));
}

void lOperationsMath(lClosure *c){
	lAddNativeFunc(c,"abs",  "[a]", "Return the absolute value of a", lnfAbs);
	lAddNativeFunc(c,"sqrt", "[a]", "Return the square root of a",    lnfSqrt);
	lAddNativeFunc(c,"cbrt", "[a]", "Return the cube root of a",      lnfCbrt);
	lAddNativeFunc(c,"floor","[a]", "Round a down",                   lnfFloor);
	lAddNativeFunc(c,"ceil", "[a]", "Round a up",                     lnfCeil);
	lAddNativeFunc(c,"round","[a]", "Round a",                        lnfRound);
	lAddNativeFunc(c,"sin",  "[a]", "Sin A",                          lnfSin);
	lAddNativeFunc(c,"cos",  "[a]", "Cos A",                          lnfCos);
	lAddNativeFunc(c,"tan",  "[a]", "Tan A",                          lnfTan);

	lAddNativeFunc(c,"vec/magnitude","[vec]","Return the magnitude of VEC", lnfVMag);
}