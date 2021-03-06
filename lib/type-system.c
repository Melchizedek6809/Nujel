/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
/*
 * In this file you will find different subroutines for casting from one type to
 * another, as well as code for determining which type would be most fitting when
 * you have to for example add two values together.
 */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

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

static lVal *requireCertainType(lClosure *c, lVal *v, lType T){
	if(unlikely((v == NULL) || (v->type != T))){
		throwTypeError(c, v, T);
	}
	return v;
}

/* Cast v to be an int without memory allocations, or return fallback */
i64 castToInt(const lVal *v, i64 fallback){
	typeswitch(v){
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
	if((a == ltFloat) || (b == ltFloat)){return ltFloat;}
	if((a == ltInt)   || (b == ltInt))  {return ltInt;}
	return ltNoAlloc;
}

i64 requireInt(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltInt)->vInt;
}

i64 requireNaturalInt(lClosure *c, lVal *v){
	i64 ret = requireInt(c,v);
	if(unlikely(ret < 0)){ lExceptionThrowValClo("type-error", "Expected a Natural int, not: ", v, c); }
	return ret;
}

lBytecodeOp requireBytecodeOp(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltBytecodeOp)->vBytecodeOp;
}

lBytecodeArray *requireBytecodeArray(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltBytecodeArr)->vBytecodeArr;
}

double requireFloat(lClosure *c, lVal *v){
	typeswitch(v){
	default:      throwTypeError(c, v, ltFloat);
	case ltFloat: return v->vFloat;
	case ltInt:   return v->vInt;
	}
}

lArray *requireArray(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltArray)->vArray;
}

lString *requireString(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltString)->vString;
}

lBuffer *requireBuffer(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltBuffer)->vBuffer;
}

lBuffer *requireMutableBuffer(lClosure *c, lVal *v){
	lBuffer *ret = requireBuffer(c, v);
	if(unlikely(ret->flags & BUFFER_IMMUTABLE)){ lExceptionThrowValClo("type-error", "Buffer is immutable", v, c); }
	return ret;
}

lBufferView *requireBufferView(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltBufferView)->vBufferView;
}

lBufferView *requireMutableBufferView(lClosure *c, lVal *v){
	lBufferView *ret = requireBufferView(c, v);
	if(unlikely(ret->flags & BUFFER_VIEW_IMMUTABLE)){ lExceptionThrowValClo("type-error", "BufferView is immutable", v, c); }
	return ret;
}

lTree *requireTree(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltTree)->vTree;
}

lTree *requireMutableTree(lClosure *c, lVal *v){
	lTree *ret = requireTree(c,v);
	if(unlikely(ret->flags & TREE_IMMUTABLE)){ lExceptionThrowValClo("type-error", "Tree is immutable", v, c); }
	return ret;
}

const lSymbol *requireSymbol(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltSymbol)->vSymbol;
}

const lSymbol *requireKeyword(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltKeyword)->vSymbol;
}

const lSymbol *requireSymbolic(lClosure *c, lVal *v){
	if(unlikely((v == NULL) || ((v->type != ltSymbol) && (v->type != ltKeyword)))){
		throwTypeError(c, v, ltSymbol);
	}
	return v->vSymbol;
}

lClosure *requireClosure(lClosure *c, lVal *v){
	if((v == NULL)
		|| !((v->type == ltLambda)
		||   (v->type == ltObject)
		||   (v->type == ltMacro))){
			lExceptionThrowValClo("type-error", "Need a closure, not: ", v, c);
			return NULL;
	}
	return v->vClosure;
}

lVal *requireCallable(lClosure *c, lVal *v){
	if((v == NULL)
		|| !((v->type == ltLambda)
		||   (v->type == ltNativeFunc)
		||   (v->type == ltMacro))){
			lExceptionThrowValClo("type-error", "Need sometihng callable, not: ", v, c);
	}
	return v;
}

lVal *requireEnvironment(lClosure *c, lVal *v){
	return requireCertainType(c, v, ltObject);
}

FILE *requireFileHandle(lClosure *c, lVal *v){
	requireCertainType(c, v, ltFileHandle);
	return v->vFileHandle;
}
