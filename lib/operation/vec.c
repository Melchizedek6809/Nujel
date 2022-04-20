/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../misc/vec.h"
#include "../type/native-function.h"
#include "../type/val.h"
#include "../exception.h"

/* [vec/x vec] - Return x part of VEC */
static lVal *lnfVX(lClosure *c, lVal *v){
	(void)c;
	const vec val = castToVec(lCar(v), vecZero());
	return lValFloat(val.x);
}

/* [vec/y vec] - Return y part of VEC */
static lVal *lnfVY(lClosure *c, lVal *v){
	(void)c;
	const vec val = castToVec(lCar(v), vecZero());
	return lValFloat(val.y);
}

/* [vec/z vec] - Return z part of VEC */
static lVal *lnfVZ(lClosure *c, lVal *v){
	(void)c;
	const vec val = castToVec(lCar(v), vecZero());
	return lValFloat(val.z);
}

static lVal *lnfVMagnitude(lClosure *c, lVal *v){
	(void)c;
	const vec val = castToVec(lCar(v), vecZero());
	return lValFloat(vecMag(val));
}

static lVal *lnfVNormalize(lClosure *c, lVal *v){
	(void)c;
	const vec val = castToVec(lCar(v), vecZero());
	return lValVec(vecNorm(val));
}

static lVal *lnfVDot(lClosure *c, lVal *v){
	(void)c;
	const vec a = castToVec(lCar(v), vecZero());
	const vec b = castToVec(lCadr(v), vecZero());
	return lValFloat(vecDot(a,b));
}

static lVal *lnfVReflect(lClosure *c, lVal *v){
	(void)c;
	const vec i = castToVec(lCar(v), vecZero());
	const vec n = castToVec(lCadr(v), vecOne());
	return lValVec(vecReflect(i,n));
}

#include "../misc/pf.h"

/* [vec v] - Convert v into a vector value consistig of 3 floats x,y and z */
lVal *lnfVec(lClosure *c, lVal *v){
	if(v == NULL){
		zeroVal:
		lExceptionThrowValClo("type-error", "[vec] needs at least one numeric value", v, c);
	}

	vec nv = vecNew(0,0,0);
	int i = 0;
	for(;i<3;i++){
		lVal *car = lCar(v);
		if(car == NULL){break;}
		switch(car->type){
		default:
			lExceptionThrowValClo("type-error", "[vec] can construct a vector out of scalar :int and :float values", v, c);
			return NULL;
		case ltFloat:
			nv.v[i] = car->vFloat;
			break;
		case ltInt:
			nv.v[i] = car->vInt;
			break;
		}
		v = lCdr(v);
	}

	if(i <= 0){ goto zeroVal; }
	for(;i<3;i++){
		nv.v[i] = nv.v[i-1];
	}
	return lValVec(nv);
}

void lOperationsVector(lClosure *c){
	lAddNativeFunc(c,"vec",           "[x y z]", "Return a new vector consisting of X,Y and Z", lnfVec);
	lAddNativeFunc(c,"vec/x",         "[vec]",   "Return x part of VEC", lnfVX);
	lAddNativeFunc(c,"vec/y",         "[vec]",   "Return y part of VEC", lnfVY);
	lAddNativeFunc(c,"vec/z",         "[vec]",   "Return z part of VEC", lnfVZ);
	lAddNativeFunc(c,"vec/dot",       "[a b]",   "Return the dot product of A and B", lnfVDot);
	lAddNativeFunc(c,"vec/magnitude", "[vec]",   "Return length of VEC", lnfVMagnitude);
	lAddNativeFunc(c,"vec/normalize", "[vec]",   "Return a normalized version of VEC", lnfVNormalize);
	lAddNativeFunc(c,"vec/reflect",   "[i n]",   "Calculate the reflection direction for an incident vector", lnfVReflect);
}
