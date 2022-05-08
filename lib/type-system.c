/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
/*
 * In this file you will find different subroutines for casting from one type to
 * another, as well as code for determining which type would be most fitting when
 * you have to for example add two values together.
 */
#include "type-system.h"
#include "allocation/symbol.h"
#include "collection/list.h"
#include "collection/string.h"
#include "display.h"
#include "exception.h"
#include "misc/pf.h"
#include "misc/vec.h"
#include "operation.h"
#include "type/closure.h"
#include "type/symbol.h"

#include <stdlib.h>


/* Cast v to be an int without memory allocations, or return fallback */
i64 castToInt(const lVal *v, i64 fallback){
	switch(v ? v->type : ltNoAlloc){
	case ltVec:
		return v->vVec.x;
	case ltFloat:
		return v->vFloat;
	case ltInt:
		return v->vInt;
	default:
		return fallback;
	}
}

/* Cast v to be a bool without memory allocations, or return false */
bool castToBool(const lVal *v){
	if(v == NULL){
		return false;
	}else{
		return v->type == ltBool ? v->vBool : true;
	}
}

const char *castToString(const lVal *v, const char *fallback){
	if((v == NULL) || (v->type != ltString)){return fallback;}
	return v->vString->data;
}

/* Determine which type has the highest precedence between a and b */
lType lTypecast(const lType a,const lType b){
	if((a == ltVec)   || (b == ltVec))  {return ltVec;}
	if((a == ltFloat) || (b == ltFloat)){return ltFloat;}
	if((a == ltInt)   || (b == ltInt))  {return ltInt;}
	if((a == ltBool)  || (b == ltBool)) {return ltBool;}
	if (a == b){ return a;}
	return ltNoAlloc;
}

i64 requireInt(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltInt)){
		lExceptionThrowValClo("type-error", "Expected an int, not: ", v, c);
	}
	return v->vInt;
}

i64 requireNaturalInt(lClosure *c, lVal *v){
	i64 ret = requireInt(c,v);
	if(ret < 0){
		lExceptionThrowValClo("type-error", "Expected a Natural int, not: ", v, c);
	}
	return ret;
}

lBytecodeOp requireBytecodeOp(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltBytecodeOp)){
		lExceptionThrowValClo("type-error", "Expected a bytecode operation, not: ", v, c);
	}
	return v->vBytecodeOp;
}

lBytecodeArray requireBytecodeArray(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltBytecodeArr)){
		lExceptionThrowValClo("type-error", "Expected a bytecode array, not: ", v, c);
	}
	return v->vBytecodeArr;
}

double requireFloat(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	default:
		lExceptionThrowValClo("type-error", "Expected a float, not: ", v, c);
	case ltFloat:
		return v->vFloat;
	case ltInt:
		return v->vInt;
	}
}

vec requireVec(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltVec)){
		lExceptionThrowValClo("type-error", "Expected a vector, not: ", v, c);
	}
	return v->vVec;
}

vec requireVecCompatible(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	case ltVec:
		return v->vVec;
	case ltFloat:
		return vecNew(v->vFloat, v->vFloat, v->vFloat);
	case ltInt:
		return vecNew(v->vInt, v->vInt, v->vInt);
	default:
		lExceptionThrowValClo("type-error", "Expected a vector, not: ", v, c);
		return v->vVec;
	}
}

lArray *requireArray(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltArray)){
		lExceptionThrowValClo("type-error", "Expected an array, not: ", v, c);
	}
	return v->vArray;
}

lString *requireString(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltString)){
		lExceptionThrowValClo("type-error", "Expected an array, not: ", v, c);
	}
	return v->vString;
}

lTree *requireTree(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltTree)){
		lExceptionThrowValClo("type-error", "Expected a tree, not: ", v, c);
	}
	return v->vTree;
}

lTree *requireMutableTree(lClosure *c, lVal *v){
	lTree *ret = requireTree(c,v);
	if(ret->flags & TREE_IMMUTABLE){
		lExceptionThrowValClo("type-error", "Tree is immutable", v, c);
	}
	return ret;
}

const lSymbol *requireSymbol(lClosure *c, lVal *v){
	if((v == NULL) || (v->type != ltSymbol)){
		lExceptionThrowValClo("type-error", "Expected a symbol, not: ", v, c);
	}
	return v->vSymbol;
}

const lSymbol *requireSymbolic(lClosure *c, lVal *v){
	if((v == NULL) || ((v->type != ltSymbol) && (v->type != ltKeyword))){
		lExceptionThrowValClo("type-error", "Expected a symbol or keyword, not: ", v, c);
	}
	return v->vSymbol;
}
