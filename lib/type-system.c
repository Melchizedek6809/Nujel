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
	v.vNFunc->args = lCar(lRead(args, strlen(args)));
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

static lVal lAddNativeStaticMethod(lClass *T, const lSymbol *name, const char *args, void *fun, uint flags, u8 argCount){
	lVal v = lValAlloc(ltNativeFunc, lNFuncAlloc());
	v.vNFunc->fp   = fun;
	v.vNFunc->args = lCar(lRead(args, strlen(args)));
	v.vNFunc->meta = NULL;
	v.vNFunc->argCount = argCount;
	if(flags & NFUNC_FOLD){
		v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symFold, lValBool(true));
	}
	if(flags & NFUNC_PURE){
		v.vNFunc->meta = lTreeInsert(v.vNFunc->meta, symPure, lValBool(true));
	}
	T->staticMethods = lTreeInsert(T->staticMethods, name, v);
	return v;
}

lVal lAddNativeStaticMethodV(lClass *T, const lSymbol *name, const char *args, lVal (*fun)(lVal), uint flags){
	return lAddNativeStaticMethod(T, name, args, fun, flags, (1 << 1));
}
lVal lAddNativeStaticMethodVV(lClass *T, const lSymbol *name, const char *args, lVal (*fun)(lVal, lVal), uint flags){
	return lAddNativeStaticMethod(T, name, args, fun, flags, (2 << 1));
}
lVal lAddNativeStaticMethodVVV(lClass *T, const lSymbol *name, const char *args, lVal (*fun)(lVal, lVal, lVal), uint flags){
	return lAddNativeStaticMethod(T, name, args, fun, flags, (3 << 1));
}

static lVal lnmTypeName(lVal self){
	if(unlikely(self.type != (self.type & 63))){
		return lValException(lSymVMError, "Out-of-bounds Type", self);
	}
	lClass *T = &lClassList[self.type];
	if(unlikely(T->name == NULL)){
		fprintf(stderr, "T: %u\n", self.type);
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

static lVal lnmAddMethod(lVal self, lVal name, lVal fn){
	reqSymbolic(name);
	if(unlikely(fn.type != ltLambda)){
		return lValExceptionType(fn, ltLambda);
	}

	self.vType->methods = lTreeInsert(self.vType->methods, name.vSymbol, fn);
	return self;
}

static lVal lnmLambdaHas(lVal self, lVal key){
	if(unlikely((key.type != ltKeyword) && (key.type != ltSymbol))){
		return lValBool(false);
	}
	lVal v = lGetClosureSym(self.vClosure, key.vSymbol);
	return lValBool(v.type != ltException);
}

static lVal lnmLambdaData(lVal self){
	return lValTree(self.vClosure->data);
}

static lVal lnmLambdaCode(lVal self){
	return lValAlloc(ltBytecodeArr, self.vClosure->text);
}

static lVal lnmLambdaArguments(lVal self){
	return self.vClosure->args;
}

static lVal lnmLambdaParent(lVal self){
	if(self.vClosure->parent == NULL){
		return NIL;
	}else{
		return lValAlloc(self.vClosure->parent->type == closureObject ? ltEnvironment : ltLambda, self.vClosure->parent);
	}
}

static lVal lnmLambdaParentSet(lVal self, lVal v){
	if(v.type == ltNil){
		self.vClosure->parent = NULL;
	} else {
		reqClosure(v);
		self.vClosure->parent = v.vClosure;
	}
	return self;
}

static lVal lnmNFuncArguments(lVal self){
	return self.vNFunc->args;
}

static void lTypesAddCoreMethods(){
	lClass *Any = &lClassList[ltAny];
	lAddNativeMethodV (Any, lSymS("type-of"), "(self)", lnmTypeOf, NFUNC_PURE);
	lAddNativeMethodV (Any, lSymS("type-name"), "(self)", lnmTypeName, NFUNC_PURE);
	lAddNativeMethodV (Any, lSymS("length"), "(self)", lnmNilLength, 0);
	lAddNativeMethodVV(Any, lSymS("meta"), "(self key)", lnmNilMetaGet, 0);

	lClass *Pair = &lClassList[ltPair];
	lAddNativeMethodV (Pair, lSymS("length"), "(self)", lnmPairLength, NFUNC_PURE);

	lClass *Lambda = &lClassList[ltLambda];
	lAddNativeMethodVV(Lambda, lSymS("has?"), "(self key)", lnmLambdaHas, NFUNC_PURE);
	lAddNativeMethodV (Lambda, lSymS("code"), "(self)", lnmLambdaCode, NFUNC_PURE);
	lAddNativeMethodV (Lambda, lSymS("data"), "(self)", lnmLambdaData, 0);
	lAddNativeMethodV (Lambda, lSymS("arguments"), "(self)", lnmLambdaArguments, 0);
	lAddNativeMethodV (Lambda, lSymS("parent"), "(self)", lnmLambdaParent, 0);
	lAddNativeMethodVV(Lambda, lSymS("parent!"), "(self v)", lnmLambdaParentSet, 0);

	lClass *NFunc = &lClassList[ltNativeFunc];
	lAddNativeMethodV (NFunc, lSymS("arguments"), "(self)", lnmNFuncArguments, 0);

	lClass *Type = &lClassList[ltType];
	lAddNativeMethodV  (Type, lSymS("name"), "(self)", lnmTName, NFUNC_PURE);
	lAddNativeMethodVVV(Type, lSymS("add-method"), "(self name fn)", lnmAddMethod, NFUNC_PURE);
}

lVal lMethodLookup(const lSymbol *method, lVal self){
	if(unlikely(self.type != (self.type & 63))){
		lValException(lSymVMError, "Out-of-bounds Type", self);
	}
	if(self.type == ltTree){
		lVal v = lTreeRef(self.vTree->root, method);
		if(v.type != ltException){
			return v;
		}
		lVal proto = lTreeRef(self.vTree->root, lSymPrototype);
		if(proto.type != ltException){
			return lMethodLookup(method, proto);
		}
	}
	if(self.type == ltType){
		const lClass *T = self.vType;
		for(;T;T = T->parent){
			for(const lTree *t = T->staticMethods; t; t = (method > t->key) ? t->right : t->left){
				if(method == t->key){
					return t->value;
				}
			}
		}
	}
	const lClass *T = &lClassList[self.type];
	for(;T;T = T->parent){
		for(const lTree *t = T->methods; t; t = (method > t->key) ? t->right : t->left){
			if(method == t->key){
				return t->value;
			}
		}
	}
	return lValException(lSymUnboundVariable, "Unbound method", self);
}

void lTypesInit(lClosure *c){
	initType(ltAny, lSymLTAny, NULL);
	lClass *tAny = &lClassList[ltAny];

	initType(ltNil, lSymLTNil, tAny);
	initType(ltSymbol, lSymLTSymbol, tAny);
	initType(ltKeyword, lSymLTKeyword, tAny);
	initType(ltBool, lSymLTBool, tAny);

	initType(ltInt, lSymLTInt, tAny);
	initType(ltFloat, lSymLTFloat, tAny);

	initType(ltPair, lSymLTPair, tAny);
	initType(ltArray, lSymLTArray, tAny);
	initType(ltTree, lSymLTTree, tAny);

	initType(ltNativeFunc, lSymLTNativeFunction, tAny);
	initType(ltLambda, lSymLTLambda, tAny);
	initType(ltMacro, lSymLTMacro, &lClassList[ltLambda]);
	initType(ltEnvironment, lSymLTEnvironment, &lClassList[ltLambda]);

	initType(ltBuffer, lSymLTBuffer, tAny);
	initType(ltBufferView, lSymLTBufferView, tAny);
	initType(ltBytecodeArr, lSymLTBytecodeArray, tAny);
	initType(ltString, lSymLTString, &lClassList[ltBuffer]);

	initType(ltFileHandle, lSymLTFileHandle, tAny);
	initType(ltType, lSymLTType, tAny);
	initType(ltComment, NULL, tAny);
	initType(ltException, NULL, tAny);

	lTypesAddCoreMethods();

	lDefineVal(c, "Nil",        lValType(&lClassList[ltNil]));
	lDefineVal(c, "Symbol",     lValType(&lClassList[ltSymbol]));
	lDefineVal(c, "Keyword",    lValType(&lClassList[ltKeyword]));
	lDefineVal(c, "Bool",       lValType(&lClassList[ltBool]));
	lDefineVal(c, "Int",        lValType(&lClassList[ltInt]));
	lDefineVal(c, "Float",      lValType(&lClassList[ltFloat]));
	lDefineVal(c, "Pair",       lValType(&lClassList[ltPair]));
	lDefineVal(c, "Array",      lValType(&lClassList[ltArray]));
	lDefineVal(c, "Tree",       lValType(&lClassList[ltTree]));
	lDefineVal(c, "Lambda",     lValType(&lClassList[ltLambda]));
	lDefineVal(c, "Macro",      lValType(&lClassList[ltMacro]));
	lDefineVal(c, "NativeFunc", lValType(&lClassList[ltNativeFunc]));
	lDefineVal(c, "Environment",lValType(&lClassList[ltEnvironment]));
	lDefineVal(c, "String",     lValType(&lClassList[ltString]));
	lDefineVal(c, "Buffer",     lValType(&lClassList[ltBuffer]));
	lDefineVal(c, "BufferView", lValType(&lClassList[ltBufferView]));
	lDefineVal(c, "BytecodeArr",lValType(&lClassList[ltBytecodeArr]));
	lDefineVal(c, "FileHandle", lValType(&lClassList[ltFileHandle]));
	lDefineVal(c, "Type",       lValType(&lClassList[ltType]));
	lDefineVal(c, "Any",        lValType(&lClassList[ltAny]));
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

lVal requireFloat(lVal v){
	if(likely(v.type == ltFloat)){
		return v;
	} else if(v.type == ltInt){
		return lValFloat(v.vInt);
	} else {
		return lValException(lSymTypeError, "Need an :Int or :Float", v);
	}
}

lVal optionalSymbolic(lVal v, const lSymbol *fallback){
	if(likely(v.type == ltNil)){
		return lValSymS(fallback);
	}
	if(unlikely((v.type != ltKeyword) && (v.type != ltSymbol))){
		return lValException(lSymTypeError, "Need a :Symbol or :Keyword", v);
	}
	return v;
}
