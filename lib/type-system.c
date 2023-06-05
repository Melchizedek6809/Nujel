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

lVal lValExceptionType(lVal v, lType T){
	char buf[128];
	snprintf(buf, sizeof(buf), "expected argument of type %s, not: ", getTypeSymbolT(T)->c);
	buf[sizeof(buf)-1] = 0;
	return lValException("type-error", buf, v);
}

lVal lValExceptionArity(lVal v, int arity){
	char buf[128];
	snprintf(buf, sizeof(buf), "This subroutine needs %i arguments", arity);
	buf[sizeof(buf)-1] = 0;
	return lValException("arity-error", buf, v);
}

lVal lValExceptionNonNumeric(lVal v){
	return lValException("type-error","Can't calculate with non numeric types", v);
}

lVal lValExceptionFloat(lVal v){
	return lValException("type-error","This function can only be used with floats",v);
}

/* Cast v to be an int without memory allocations, or return fallback */
i64 castToInt(const lVal v, i64 fallback){
	switch(v.type){
		case ltFloat: return v.vFloat;
		case ltInt:   return v.vInt;
		default:      return fallback;
	}
}

/* Cast v to be a bool without memory allocations, or return false */
bool castToBool(const lVal v){
	return (v.type == ltBool ? v.vBool : likely(v.type != ltNil));
}

const char *castToString(const lVal v, const char *fallback){
	return (v.type != ltString) ? fallback : v.vString->data;
}

/* Determine which type has the highest precedence between a and b */
lType lTypecast(const lType a, const lType b){
	if (likely(a == b)){
		return a;
	}
	if((a == ltFloat) || (b == ltFloat)){
		return ltFloat;
	}
	return ltNil;
}

lVal requireInt(lVal v){
	if(unlikely(v.type != ltInt)){
		return lValException("type-error", "Need an :Int", v);
	}
	return v;
}

lVal requireNaturalInt(lVal v){
	if(unlikely(v.type != ltInt)){
		return lValException("type-error", "Need an :Int", v);
	}
	if(unlikely(v.vInt < 0)){
		return lValException("type-error", "Expected a Natural int, not: ", v);
	}
	return v;
}

lVal requireBytecodeArray(lVal v){
	if(unlikely(v.type != ltBytecodeArr)){
		return lValException("type-error", "Need an :BytecodeArray", v);
	}
	return v;
}

lVal requireFloat(lVal v){
	if(likely(v.type == ltFloat)){
		return v;
	} else if(v.type == ltInt){
		return lValFloat(v.vInt);
	} else {
		return lValException("type-error", "Need an :Int or :Float", v);
	}
}

lVal requireArray(lVal v){
	if(unlikely(v.type != ltArray)){
		return lValException("type-error", "Need an :Array", v);
	}
	return v;
}

lVal requireMutableArray(lVal v){
	if(unlikely(v.type != ltArray)){
		return lValException("type-error", "Need an :Array", v);
	}
	if(unlikely(v.vArray->flags & ARRAY_IMMUTABLE)){
		return lValException("type-error", "The provided array is immutable", v);
	}
	return v;
}

lVal requireString(lVal v){
	if(unlikely(v.type != ltString)){
		return lValException("type-error", "Need a :String", v);
	}
	return v;
}

lVal requireBuffer(lVal v){
	if(unlikely(v.type != ltBuffer)){
		return lValException("type-error", "Need a :Buffer", v);
	}
	return v;
}

lVal requireMutableBuffer(lVal v){
	if(unlikely(v.type != ltBuffer)){
		return lValException("type-error", "Need a :Buffer", v);
	}
	if(unlikely(v.vBuffer->flags & BUFFER_IMMUTABLE)){
		return lValException("type-error", "Buffer is immutable", v);
	}
	return v;
}

lVal requireBufferView(lVal v){
	if(unlikely(v.type != ltBufferView)){
		return lValException("type-error", "Need a :BufferView", v);
	}
	return v;
}

lVal requireMutableBufferView(lVal v){
	if(unlikely(v.type != ltBufferView)){
		return lValException("type-error", "Need a :BufferView", v);
	}
	if(unlikely(v.vBufferView->flags & BUFFER_VIEW_IMMUTABLE)){
		return lValException("type-error", "BufferView is immutable", v);
	}
	return v;
}

lVal requireTree(lVal v){
	if(unlikely(v.type != ltTree)){
		return lValException("type-error", "Need a :Tree", v);
	}
	return v;
}

lVal requireMutableTree(lVal v){
	if(unlikely(v.type != ltTree)){
		return lValException("type-error", "Need a :Tree", v);
	}
	if(unlikely(v.vTree->root && v.vTree->root->flags & TREE_IMMUTABLE)){
		return lValException("type-error", "Tree is immutable", v);
	}
	return v;
}

lVal requireSymbol(lVal v){
	if(unlikely(v.type != ltSymbol)){
		return lValException("type-error", "Need a :Symbol", v);
	}
	return v;
}

lVal requireKeyword(lVal v){
	if(unlikely(v.type != ltKeyword)){
		return lValException("type-error", "Need a :Keyword", v);
	}
	return v;
}

lVal requireSymbolic(lVal v){
	if(unlikely((v.type != ltKeyword) && (v.type != ltSymbol))){
		return lValException("type-error", "Need a :Symbol or :Keyword", v);
	}
	return v;
}

lVal optionalSymbolic(lVal v, const lSymbol *fallback){
	if(likely(v.type == ltNil)){
		return lValSymS(fallback);
	}
	return requireSymbolic(v);
}

lVal requireClosure(lVal v){
	if(unlikely(!((v.type == ltLambda)
		|| (v.type == ltEnvironment)
		|| (v.type == ltMacro))))
	{
			return lValException("type-error", "Need a closure, not: ", v);
	}
	return v;
}

lVal requireCallable(lVal v){
	if(unlikely(!((v.type == ltLambda)
		|| (v.type == ltNativeFunc)
		|| (v.type == ltMacro))))
	{
			return lValException("type-error", "Need something callable, not: ", v);
	}
	return v;
}

lVal requireFileHandle(lVal v){
	if(unlikely(v.type != ltFileHandle)) {
		return lValException("type-error", "Need a :FileHandle", v);
	}
	return v;
}

lVal requirePair(lVal v){
	if(unlikely(v.type != ltPair)){
		return lValException("type-error", "Need a :Pair", v);
	}
	return v;
}
