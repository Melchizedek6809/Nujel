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

lClass lClassList[64];

static void initType(int i, const lSymbol *name, lClass *parent){
	lClassList[i].name = name;
	lClassList[i].parent = parent;
	lClassList[i].methods = NULL;
	lClassList[i].staticMethods = NULL;
}

static lVal lAddNativeMethod(lClass *T, const lSymbol *name, const char *args, void *fun, uint flags, u8 argCount){
	lVal v = lValAlloc(ltNativeFunc, lNFuncAlloc());
	v.vNFunc->fp   = fun;
	v.vNFunc->args = lCar(lRead(args));
	v.vNFunc->meta = NULL;
	v.vNFunc->argCount = argCount;
	if(flags & NFUNC_FOLD){
		v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symFold, lValBool(true));
	}
	if(flags & NFUNC_PURE){
		v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symPure, lValBool(true));
	}
	T->methods = lTreeInsert(T->methods, name, v);
	return v;
}

lVal lAddNativeMethodV(lClass *T, const lSymbol *name, const char *args, lVal (*fun)(lVal), uint flags){
	return lAddNativeMethod(T, name, args, fun, flags, (1 << 1));
}
lVal lAddNativeMethodVV(lClass *T, const lSymbol *name, const char *args, lVal (*fun)(lVal, lVal), uint flags){
	return lAddNativeMethod(T, name, args, fun, flags, (2 << 1));
}
lVal lAddNativeMethodVVV(lClass *T, const lSymbol *name, const char *args, lVal (*fun)(lVal, lVal, lVal), uint flags){
	return lAddNativeMethod(T, name, args, fun, flags, (3 << 1));
}

static lVal lnmTypeName(lVal self){
	if(unlikely(self.type != (self.type & 63))){
		return lValException(lSymVMError, "Out-of-bounds Type", self);
	}
	lClass *T = &lClassList[self.type];
	if(unlikely(T->name == NULL)){
		printf("T: %u\n", self.type);
		return lValException(lSymVMError, "Unnamed Type", self);
	}
	return lValKeywordS(T->name);
}

static lVal lnmTypeOf(lVal self){
	if(unlikely(self.type != (self.type & 63))){
		return lValException(lSymVMError, "Out-of-bounds Type", self);
	}
	return lValType(&lClassList[self.type]);
}

static lVal lnmNilMetaGet(lVal self, lVal key){
	(void)self;(void)key;
	return NIL;
}

static lVal lnmNilLength(lVal self){
	(void)self;
	return lValInt(0);
}

static lVal lnmPairLength(lVal self){
	lVal l = self;
	int i = 0;
	for(; l.type == ltPair; l = l.vList->cdr){
		i++;
	}
	if(unlikely(l.type != ltNil)){
		i++;
	}
	return lValInt(i);
}

static lVal lnmTName(lVal self){
	if(unlikely(self.vType->name == NULL)){
		return lValException(lSymVMError, "Unnamed Type", self);
	}
	return lValKeywordS(self.vType->name);
}

static void lTypesAddCoreMethods(){
	lClass *Nil = &lClassList[ltNil];
	lAddNativeMethodV (Nil, lSymS("type-of"), "(self)", lnmTypeOf, NFUNC_PURE);
	lAddNativeMethodV (Nil, lSymS("type-name"), "(self)", lnmTypeName, NFUNC_PURE);
	lAddNativeMethodV (Nil, lSymS("length"), "(self)", lnmNilLength, 0);
	lAddNativeMethodVV(Nil, lSymS("meta"), "(self key)", lnmNilMetaGet, 0);

	lClass *Pair = &lClassList[ltPair];
	lAddNativeMethodV (Pair, lSymS("length"), "(self)", lnmPairLength, NFUNC_PURE);

	lClass *Type = &lClassList[ltType];
	lAddNativeMethodV (Type, lSymS("name"), "(self)", lnmTName, NFUNC_PURE);
}

lVal lMethodLookup(const lSymbol *method, lVal self){
	if(unlikely(self.type != (self.type & 63))){
		lValException(lSymVMError, "Out-of-bounds Type", self);
	}
	lClass *T = &lClassList[self.type];
	for(;T;T = T->parent){
		const lTree *t = T->methods;
		while(t){
			if(method == t->key){
				return t->value;
			}
			t = method > t->key ? t->right : t->left;
		}
	}
	return lValException(lSymUnboundVariable, "Unbound method", self);
}

void lTypesInit(){
	initType(ltNil, lSymLTNil, NULL);
	lClass *tNil = &lClassList[ltNil];
	initType(ltSymbol, lSymLTSymbol, tNil);
	initType(ltKeyword, lSymLTKeyword, tNil);
	initType(ltBool, lSymLTBool, tNil);

	initType(ltInt, lSymLTInt, tNil);
	initType(ltFloat, lSymLTFloat, tNil);

	initType(ltPair, lSymLTPair, tNil);
	initType(ltArray, lSymLTArray, tNil);
	initType(ltTree, lSymLTTree, tNil);

	initType(ltNativeFunc, lSymLTNativeFunction, tNil);
	initType(ltLambda, lSymLTLambda, tNil);
	initType(ltMacro, lSymLTMacro, &lClassList[ltLambda]);
	initType(ltEnvironment, lSymLTEnvironment, &lClassList[ltLambda]);

	initType(ltBuffer, lSymLTBuffer, tNil);
	initType(ltBufferView, lSymLTBufferView, tNil);
	initType(ltBytecodeArr, lSymLTBytecodeArray, tNil);
	initType(ltString, lSymLTString, &lClassList[ltBuffer]);

	initType(ltFileHandle, lSymLTFileHandle, tNil);
	initType(ltType, lSymLTType, tNil);
	initType(ltComment, NULL, NULL);
	initType(ltException, NULL, NULL);

	lTypesAddCoreMethods();
}

lVal lValExceptionType(lVal v, lType T){
	char buf[128];
	snprintf(buf, sizeof(buf), "expected argument of type %s, not: ", getTypeSymbolT(T)->c);
	buf[sizeof(buf)-1] = 0;
	return lValException(lSymTypeError, buf, v);
}

lVal lValExceptionArity(lVal v, int arity){
	char buf[128];
	snprintf(buf, sizeof(buf), "This subroutine needs %i arguments", arity);
	buf[sizeof(buf)-1] = 0;
	return lValException(lSymArityError, buf, v);
}

lVal lValExceptionNonNumeric(lVal v){
	return lValException(lSymTypeError, "Can't calculate with non numeric types", v);
}

lVal lValExceptionFloat(lVal v){
	return lValException(lSymTypeError, "This function can only be used with floats",v);
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
		return lValException(lSymTypeError, "Need an :Int", v);
	}
	return v;
}

lVal requireNaturalInt(lVal v){
	if(unlikely(v.type != ltInt)){
		return lValException(lSymTypeError, "Need an :Int", v);
	}
	if(unlikely(v.vInt < 0)){
		return lValException(lSymTypeError, "Expected a Natural int, not: ", v);
	}
	return v;
}

lVal requireBytecodeArray(lVal v){
	if(unlikely(v.type != ltBytecodeArr)){
		return lValException(lSymTypeError, "Need an :BytecodeArray", v);
	}
	return v;
}

lVal requireFloat(lVal v){
	if(likely(v.type == ltFloat)){
		return v;
	} else if(v.type == ltInt){
		return lValFloat(v.vInt);
	} else {
		return lValException(lSymTypeError, "Need an :Int or :Float", v);
	}
}

lVal requireArray(lVal v){
	if(unlikely(v.type != ltArray)){
		return lValException(lSymTypeError, "Need an :Array", v);
	}
	return v;
}

lVal requireMutableArray(lVal v){
	if(unlikely(v.type != ltArray)){
		return lValException(lSymTypeError, "Need an :Array", v);
	}
	if(unlikely(v.vArray->flags & ARRAY_IMMUTABLE)){
		return lValException(lSymTypeError, "The provided array is immutable", v);
	}
	return v;
}

lVal requireString(lVal v){
	if(unlikely(v.type != ltString)){
		return lValException(lSymTypeError, "Need a :String", v);
	}
	return v;
}

lVal requireBuffer(lVal v){
	if(unlikely(v.type != ltBuffer)){
		return lValException(lSymTypeError, "Need a :Buffer", v);
	}
	return v;
}

lVal requireMutableBuffer(lVal v){
	if(unlikely(v.type != ltBuffer)){
		return lValException(lSymTypeError, "Need a :Buffer", v);
	}
	if(unlikely(v.vBuffer->flags & BUFFER_IMMUTABLE)){
		return lValException(lSymTypeError, "Buffer is immutable", v);
	}
	return v;
}

lVal requireBufferView(lVal v){
	if(unlikely(v.type != ltBufferView)){
		return lValException(lSymTypeError, "Need a :BufferView", v);
	}
	return v;
}

lVal requireMutableBufferView(lVal v){
	if(unlikely(v.type != ltBufferView)){
		return lValException(lSymTypeError, "Need a :BufferView", v);
	}
	if(unlikely(v.vBufferView->flags & BUFFER_VIEW_IMMUTABLE)){
		return lValException(lSymTypeError, "BufferView is immutable", v);
	}
	return v;
}

lVal requireTree(lVal v){
	if(unlikely(v.type != ltTree)){
		return lValException(lSymTypeError, "Need a :Tree", v);
	}
	return v;
}

lVal requireMutableTree(lVal v){
	if(unlikely(v.type != ltTree)){
		return lValException(lSymTypeError, "Need a :Tree", v);
	}
	if(unlikely(v.vTree->root && v.vTree->root->flags & TREE_IMMUTABLE)){
		return lValException(lSymTypeError, "Tree is immutable", v);
	}
	return v;
}

lVal requireSymbol(lVal v){
	if(unlikely(v.type != ltSymbol)){
		return lValException(lSymTypeError, "Need a :Symbol", v);
	}
	return v;
}

lVal requireKeyword(lVal v){
	if(unlikely(v.type != ltKeyword)){
		return lValException(lSymTypeError, "Need a :Keyword", v);
	}
	return v;
}

lVal requireSymbolic(lVal v){
	if(unlikely((v.type != ltKeyword) && (v.type != ltSymbol))){
		return lValException(lSymTypeError, "Need a :Symbol or :Keyword", v);
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
			return lValException(lSymTypeError, "Need a closure, not: ", v);
	}
	return v;
}

lVal requireCallable(lVal v){
	if(unlikely(!((v.type == ltLambda)
		|| (v.type == ltNativeFunc)
		|| (v.type == ltMacro))))
	{
			return lValException(lSymTypeError, "Need something callable, not: ", v);
	}
	return v;
}

lVal requireFileHandle(lVal v){
	if(unlikely(v.type != ltFileHandle)) {
		return lValException(lSymTypeError, "Need a :FileHandle", v);
	}
	return v;
}

lVal requirePair(lVal v){
	if(unlikely(v.type != ltPair)){
		return lValException(lSymTypeError, "Need a :Pair", v);
	}
	return v;
}
