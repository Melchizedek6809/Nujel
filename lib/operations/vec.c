/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../type/closure.h"
#include "../type/val.h"

static lVal *lnfVX(lClosure *c, lVal *v){
	return lValFloat(requireVec(c, lCar(v)).x);
}

static lVal *lnfVY(lClosure *c, lVal *v){
	return lValFloat(requireVec(c, lCar(v)).y);
}

static lVal *lnfVZ(lClosure *c, lVal *v){
	return lValFloat(requireVec(c, lCar(v)).z);
}

static lVal *lnfVW(lClosure *c, lVal *v){
	return lValFloat(requireVec(c, lCar(v)).w);
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

lVal *lnfVec(lClosure *c, lVal *v){
	vec nv = vecNew(0,0,0,0);
	if(v == NULL){return lValVec(nv);}
	if(v->type != ltPair){
		if(v->type == ltInt){
			return lValVec(vecNew(v->vInt, v->vInt, v->vInt, v->vInt));
		}else if(v->type == ltFloat){
			return lValVec(vecNew(v->vFloat, v->vFloat, v->vFloat, v->vFloat));
		}else if(v->type == ltVec){
			return v;
		}
	}
	int i = 0;
	for(lVal *cv = v; cv && cv->type == ltPair; cv = cv->vList.cdr){
		lVal *t = lCar(cv);
		if(t == NULL){break;}
		switch(t->type){
		case ltInt:
			nv.v[i] = t->vInt;
			break;
		case ltFloat:
			nv.v[i] = t->vFloat;
			break;
		case ltVec:
			if(i == 0){return t;}
			lExceptionThrowValClo("type-error", "vectors can't contain other vectors, only :float and :int values", t, c);
		default:
			lExceptionThrowValClo("type-error", "Unexpected value in [vec]", t, c);
			break;
		}
		if(++i >= 4){break;}
	}
	for(int ii=MAX(1,i);ii<4;ii++){
		nv.v[ii] = nv.v[ii-1];
	}
	return lValVec(nv);
}

void lOperationsVector(lClosure *c){
	lAddNativeFuncPure(c,"vec",           "[x y z w]", "Convert α into a vector value consisting of 4 floats x,y,z and w", lnfVec);
	lAddNativeFuncPure(c,"vec/x",         "[vec]",     "Return x part of VEC", lnfVX);
	lAddNativeFuncPure(c,"vec/y",         "[vec]",     "Return y part of VEC", lnfVY);
	lAddNativeFuncPure(c,"vec/z",         "[vec]",     "Return z part of VEC", lnfVZ);
	lAddNativeFuncPure(c,"vec/w",         "[vec]",     "Return z part of VEC", lnfVW);
	lAddNativeFuncPure(c,"vec/dot",       "[a b]",     "Return the dot product of A and B", lnfVecDot);
	lAddNativeFuncPure(c,"vec/magnitude", "[a]",       "Return the magnitude of vector A", lnfVecMagnitude);
	lAddNativeFuncPure(c,"vec/sum",       "[a]",       "Return the sum of vector A", lnfVecSum);
	lAddNativeFuncPure(c,"vec/sum/abs",   "[a]",       "Return the absolute sum of vector A", lnfVecSumAbs);
	lAddNativeFuncPure(c,"vec/cross",     "[a b]",     "Return the cross of A and B", lnfVecCross);
	lAddNativeFuncPure(c,"vec/rotate",    "[a b rad]", "Return A around axis B with RAD radians", lnfVecRotate);
	lAddNativeFuncPure(c,"vec/normalize", "[a]",       "Return a normalized version of A", lnfVecNormalize);
	lAddNativeFuncPure(c,"vec/reflect",   "[i n]",     "Calculate the reflection direction for an incident vector", lnfVecReflect);
	lAddNativeFuncPure(c,"vec/vel->rot",  "[a]",       "Return a rotation vector for the velocity vector A", lnfVecVelToRot);
	lAddNativeFuncPure(c,"vec/rot->vel",  "[a]",       "Return a velocity vector in the direction of rotation vector A", lnfVecRotToVel);
}
