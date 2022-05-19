/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
/*
 * In this file you will find different subroutines for casting from one type to
 * another, as well as code for determining which type would be most fitting when
 * you have to for example add two values together.
 */
#include "type-system.h"
#include "allocation/symbol.h"
#include "misc/pf.h"

NORETURN void throwTypeError(lClosure *c, lVal *v, lType T){
	char buf[128];
	spf(buf, &buf[sizeof(buf)], "expected argument of type %s, not: ", getTypeSymbolT(T)->c);
	lExceptionThrowValClo("type-error", buf, v, c);
}

NORETURN void throwArityError(lClosure *c, lVal *v, int arity){
	char buf[128];
	spf(buf, &buf[sizeof(buf)], "This subroutine needs %i arguments", (i64)arity);
	lExceptionThrowValClo("arity-error", buf, v, c);
}

void requireCertainType(lClosure *c, lVal *v, lType T){
	if((v == NULL) || (v->type != T)){
		throwTypeError(c, v, T);
	}
}

/* Cast v to be an int without memory allocations, or return fallback */
i64 castToInt(const lVal *v, i64 fallback){
	switch(v ? v->type : ltNoAlloc){
		case ltFloat: return v->vFloat;
		case ltInt:   return v->vInt;
		default:      return fallback;
	}
}

/* Cast v to be a bool without memory allocations, or return false */
bool castToBool(const lVal *v){
	return !v ? false : (v->type == ltBool ? v->vBool : true);
}

const char *castToString(const lVal *v, const char *fallback){
	return ((v == NULL) || (v->type != ltString)) ? fallback : v->vString->data;
}

/* Determine which type has the highest precedence between a and b */
lType lTypecast(const lType a, const lType b){
	if (a == b){ return a;}
	if((a == ltVec)   || (b == ltVec))  {return ltVec;}
	if((a == ltFloat) || (b == ltFloat)){return ltFloat;}
	if((a == ltInt)   || (b == ltInt))  {return ltInt;}
	return ltNoAlloc;
}

i64 requireInt(lClosure *c, lVal *v){
	requireCertainType(c, v, ltInt);
	return v->vInt;
}

i64 requireNaturalInt(lClosure *c, lVal *v){
	i64 ret = requireInt(c,v);
	if(ret < 0){ lExceptionThrowValClo("type-error", "Expected a Natural int, not: ", v, c); }
	return ret;
}

lBytecodeOp requireBytecodeOp(lClosure *c, lVal *v){
	requireCertainType(c, v, ltBytecodeOp);
	return v->vBytecodeOp;
}

lBytecodeArray *requireBytecodeArray(lClosure *c, lVal *v){
	requireCertainType(c, v, ltBytecodeArr);
	return v->vBytecodeArr;
}

double requireFloat(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	default:
		throwTypeError(c, v, ltFloat);
	case ltFloat:
		return v->vFloat;
	case ltInt:
		return v->vInt;
	}
}

vec requireVec(lClosure *c, lVal *v){
	requireCertainType(c, v, ltVec);
	return v->vVec;
}

vec requireVecCompatible(lClosure *c, lVal *v){
	switch(v ? v->type : ltNoAlloc){
	default:
		throwTypeError(c, v, ltVec);
	case ltVec:
		return v->vVec;
	case ltFloat:
		return vecNew(v->vFloat, v->vFloat, v->vFloat);
	case ltInt:
		return vecNew(v->vInt, v->vInt, v->vInt);
	}
}

lArray *requireArray(lClosure *c, lVal *v){
	requireCertainType(c, v, ltArray);
	return v->vArray;
}

lString *requireString(lClosure *c, lVal *v){
	requireCertainType(c, v, ltString);
	return v->vString;
}

lTree *requireTree(lClosure *c, lVal *v){
	requireCertainType(c, v, ltTree);
	return v->vTree;
}

lTree *requireMutableTree(lClosure *c, lVal *v){
	lTree *ret = requireTree(c,v);
	if(ret->flags & TREE_IMMUTABLE){ lExceptionThrowValClo("type-error", "Tree is immutable", v, c); }
	return ret;
}

const lSymbol *requireSymbol(lClosure *c, lVal *v){
	requireCertainType(c, v, ltSymbol);
	return v->vSymbol;
}

const lSymbol *requireKeyword(lClosure *c, lVal *v){
	requireCertainType(c, v, ltKeyword);
	return v->vSymbol;
}

const lSymbol *requireSymbolic(lClosure *c, lVal *v){
	if((v == NULL) || ((v->type != ltSymbol) && (v->type != ltKeyword))){
		throwTypeError(c, v, ltSymbol);
	}
	return v->vSymbol;
}

lClosure *requireClosure(lClosure *c, lVal *v){
	if((v == NULL)
		|| !((v->type == ltLambda)
		||   (v->type == ltObject)
		||   (v->type == ltMacro))){
			lExceptionThrowValClo("type-error", "Can't get metadata from that value: ", v, c);
			return NULL;
	}
	return v->vClosure;
}
