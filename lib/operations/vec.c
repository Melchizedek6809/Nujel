/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "vec.h"
#include "../vec.h"
#include "../casting.h"
#include "../datatypes/native-function.h"
#include "../datatypes/list.h"
#include "../datatypes/val.h"
#include "../datatypes/vec.h"

static lVal *lnfVX(lClosure *c, lVal *v){
	(void)c;
	const vec val = castToVec(lCar(v), vecZero());
	return lValFloat(val.x);
}

static lVal *lnfVY(lClosure *c, lVal *v){
	(void)c;
	const vec val = castToVec(lCar(v), vecZero());
	return lValFloat(val.y);
}

static lVal *lnfVZ(lClosure *c, lVal *v){
	(void)c;
	const vec val = castToVec(lCar(v), vecZero());
	return lValFloat(val.z);
}

void lOperationsVector(lClosure *c){
	lAddNativeFunc(c,"vec/x","[vec]","Return x part of VEC",lnfVX);
	lAddNativeFunc(c,"vec/y","[vec]","Return y part of VEC",lnfVY);
	lAddNativeFunc(c,"vec/z","[vec]","Return z part of VEC",lnfVZ);
}
