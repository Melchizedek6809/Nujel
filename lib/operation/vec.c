/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../collection/list.h"
#include "../type/closure.h"

static lVal *lnfVX(lClosure *c, lVal *v){
	return lValFloat(requireVec(c, lCar(v)).x);
}

static lVal *lnfVY(lClosure *c, lVal *v){
	return lValFloat(requireVec(c, lCar(v)).y);
}

static lVal *lnfVZ(lClosure *c, lVal *v){
	return lValFloat(requireVec(c, lCar(v)).z);
}

static lVal *lnfVecDot(lClosure *c, lVal *v){
	const vec a = requireVec(c, lCar(v));
	const vec b = requireVec(c, lCadr(v));
	return lValFloat(vecDot(a,b));
}

static lVal *lnfVecMagnitude(lClosure *c, lVal *v){
	const vec a = requireVec(c, lCar(v));
	return lValFloat(vecMag(a));
}

static lVal *lnfVecSum(lClosure *c, lVal *v){
	const vec a = requireVec(c, lCar(v));
	return lValFloat(vecSum(a));
}

static lVal *lnfVecSumAbs(lClosure *c, lVal *v){
	const vec a = requireVec(c, lCar(v));
	return lValFloat(vecAbsSum(a));
}

static lVal *lnfVecCross(lClosure *c, lVal *v){
	const vec a = requireVec(c, lCar(v));
	const vec b = requireVec(c, lCadr(v));
	return lValVec(vecCross(a,b));
}

static lVal *lnfVecRotate(lClosure *c, lVal *v){
	const vec a = requireVec(c, lCar(v));
	const vec b = requireVec(c, lCadr(v));
	const float rad = requireFloat(c, lCaddr(v));
	return lValVec(vecRotate(a,b,rad));
}

static lVal *lnfVecNormalize(lClosure *c, lVal *v){
	const vec a = requireVec(c, lCar(v));
	return lValVec(vecNorm(a));
}

static lVal *lnfVecVelToRot(lClosure *c, lVal *v){
	const vec a = requireVec(c, lCar(v));
	return lValVec(vecVecToDeg(a));
}

static lVal *lnfVecRotToVel(lClosure *c, lVal *v){
	const vec a = requireVec(c, lCar(v));
	return lValVec(vecDegToVec(a));
}

static lVal *lnfVecReflect(lClosure *c, lVal *v){
	const vec i = requireVec(c, lCar(v));
	const vec n = requireVec(c, lCadr(v));
	return lValVec(vecReflect(i,n));
}

void lOperationsVector(lClosure *c){
	lAddNativeFunc(c,"vec/x",         "[vec]",     "Return x part of VEC", lnfVX);
	lAddNativeFunc(c,"vec/y",         "[vec]",     "Return y part of VEC", lnfVY);
	lAddNativeFunc(c,"vec/z",         "[vec]",     "Return z part of VEC", lnfVZ);
	lAddNativeFunc(c,"vec/dot",       "[a b]",     "Return the dot product of A and B", lnfVecDot);
	lAddNativeFunc(c,"vec/magnitude", "[a]",       "Return the magnitude of vector A", lnfVecMagnitude);
	lAddNativeFunc(c,"vec/sum",       "[a]",       "Return the sum of vector A", lnfVecSum);
	lAddNativeFunc(c,"vec/sum/abs",   "[a]",       "Return the absolute sum of vector A", lnfVecSumAbs);
	lAddNativeFunc(c,"vec/cross",     "[a b]",     "Return the cross of A and B", lnfVecCross);
	lAddNativeFunc(c,"vec/rotate",    "[a b rad]", "Return A around axis B with RAD radians", lnfVecRotate);
	lAddNativeFunc(c,"vec/normalize", "[a]",       "Return a normalized version of A", lnfVecNormalize);
	lAddNativeFunc(c,"vec/reflect",   "[i n]",     "Calculate the reflection direction for an incident vector", lnfVecReflect);
	lAddNativeFunc(c,"vec/vel->rot",  "[a]",       "Return a rotation vector for the velocity vector A", lnfVecVelToRot);
	lAddNativeFunc(c,"vec/rot->vel",  "[a]",       "Return a velocity vector in the direction of rotation vector A", lnfVecRotToVel);
}
