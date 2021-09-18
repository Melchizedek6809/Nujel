/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "vec.h"
#include "../datatypes/native-function.h"
#include "../datatypes/list.h"
#include "../datatypes/val.h"
#include "../datatypes/vec.h"

static lVal *lnfVX(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastSpecific(c,v,ltVec));
	if((t == NULL) || (t->type != ltVec)){return lValFloat(0);}
	return lValFloat(lVecV(t->vCdr).x);
}

static lVal *lnfVY(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastSpecific(c,v,ltVec));
	if((t == NULL) || (t->type != ltVec)){return lValFloat(0);}
	return lValFloat(lVecV(t->vCdr).y);
}

static lVal *lnfVZ(lClosure *c, lVal *v){
	lVal *t = lCar(lEvalCastSpecific(c,v,ltVec));
	if((t == NULL) || (t->type != ltVec)){return lValFloat(0);}
	return lValFloat(lVecV(t->vCdr).z);
}

void lOperationsVector(lClosure *c){
	lAddNativeFunc(c,"vec/x","[vec]","Return x part of VEC",lnfVX);
	lAddNativeFunc(c,"vec/y","[vec]","Return y part of VEC",lnfVY);
	lAddNativeFunc(c,"vec/z","[vec]","Return z part of VEC",lnfVZ);
}
