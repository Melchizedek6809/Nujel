/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

lMap *lSymbolTable;

lSymbol  lSymbolList[SYM_MAX];
uint     lSymbolActive = 0;
uint     lSymbolMax    = 0;
lSymbol *lSymbolFFree = NULL;

lSymbol *symNull;
lSymbol *symQuote;
lSymbol *symQuasiquote;
lSymbol *symUnquote;
lSymbol *symUnquoteSplicing;
lSymbol *symArr;
lSymbol *symTreeNew;
lSymbol *symMapNew;
lSymbol *symDocumentation;
lSymbol *symPure;
lSymbol *symFold;
lSymbol *symRef;

lSymbol *symType;
lSymbol *symArguments;
lSymbol *symCode;
lSymbol *symData;
lSymbol *symName;

lSymbol *lSymLTNil;
lSymbol *lSymLTBool;
lSymbol *lSymLTPair;
lSymbol *lSymLTLambda;
lSymbol *lSymLTInt;
lSymbol *lSymLTFloat;
lSymbol *lSymLTString;
lSymbol *lSymLTSymbol;
lSymbol *lSymLTKeyword;
lSymbol *lSymLTNativeFunction;
lSymbol *lSymLTEnvironment;
lSymbol *lSymLTMacro;
lSymbol *lSymLTArray;
lSymbol *lSymLTTree;
lSymbol *lSymLTMap;
lSymbol *lSymLTBytecodeArray;
lSymbol *lSymLTBuffer;
lSymbol *lSymLTBufferView;
lSymbol *lSymLTFileHandle;
lSymbol *lSymLTUnknownType;
lSymbol *lSymLTType;
lSymbol *lSymLTAny;
lSymbol *lSymPrototype;

lSymbol *lSymFloatNaN;
lSymbol *lSymFloatInf;
lSymbol *lSymVMError;
lSymbol *lSymTypeError;
lSymbol *lSymOutOfBounds;
lSymbol *lSymIOError;
lSymbol *lSymArityError;
lSymbol *lSymDivisionByZero;
lSymbol *lSymReadError;
lSymbol *lSymOOM;
lSymbol *lSymUnmatchedOpeningBracket;
lSymbol *lSymUnboundVariable;
lSymbol *lSymNotSupportedOnPlatform;

void lSymbolInit(){
	lSymbolTable = lMapAllocRaw();
	lSymbolActive = 0;
	lSymbolMax    = 0;
	lSymbolFFree  = NULL;

	lSymbolList[0].nextFree = NULL;
	lSymbolList[0].c[sizeof(lSymbolList[0].c) - 1] = 0xFF;
	if(lSymbolList[0].nextFree != NULL){
		exit(110);
	}

	symNull              = lSymSM("");
	symQuote             = lSymSM("quote");
	symQuasiquote        = lSymSM("quasiquote");
	symUnquote           = lSymSM("unquote");
	symUnquoteSplicing   = lSymSM("unquote-splicing");
	symArr               = lSymSM("array/new");
	symTreeNew           = lSymSM("tree/new");
	symMapNew            = lSymSM("map/new");
	symDocumentation     = lSymSM("documentation");
	symPure              = lSymSM("pure");
	symFold              = lSymSM("fold");
	symRef               = lSymSM("ref");

	symType              = lSymSM("type");
	symArguments         = lSymSM("arguments");
	symCode              = lSymSM("code");
	symData              = lSymSM("data");
	symName              = lSymSM("name");

	lSymLTNil            = lSymSM("nil");
	lSymLTBool           = lSymSM("bool");
	lSymLTPair           = lSymSM("pair");
	lSymLTEnvironment    = lSymSM("environment");
	lSymLTLambda         = lSymSM("lambda");
	lSymLTInt            = lSymSM("int");
	lSymLTFloat          = lSymSM("float");
	lSymLTString         = lSymSM("string");
	lSymLTSymbol         = lSymSM("symbol");
	lSymLTKeyword        = lSymSM("keyword");
	lSymLTNativeFunction = lSymSM("native-function");
	lSymLTArray          = lSymSM("array");
	lSymLTMacro          = lSymSM("macro");
	lSymLTTree           = lSymSM("tree");
	lSymLTMap            = lSymSM("map");
	lSymLTBytecodeArray  = lSymSM("bytecode-array");
	lSymLTBuffer         = lSymSM("buffer");
	lSymLTBufferView     = lSymSM("buffer-view");
	lSymLTFileHandle     = lSymSM("file-handle");
	lSymLTUnknownType    = lSymSM("unknown-type");
	lSymLTType           = lSymSM("type");
	lSymLTAny            = lSymSM("any");

	lSymPrototype        = lSymSM("prototype*");

	lSymFloatNaN         = lSymSM("float-nan");
	lSymFloatInf         = lSymSM("float-inf");
	lSymVMError          = lSymSM("vm-error");
	lSymTypeError        = lSymSM("type-error");
	lSymOutOfBounds      = lSymSM("out-of-bounds");
	lSymIOError          = lSymSM("io-error");
	lSymArityError       = lSymSM("arity-error");
	lSymDivisionByZero   = lSymSM("division-by-zero");
	lSymReadError        = lSymSM("read-error");
	lSymOOM              = lSymSM("out-of-memory");
	lSymUnboundVariable  = lSymSM("unbound-variable");
	lSymUnmatchedOpeningBracket = lSymSM("unmatched-opening-bracket");
	lSymNotSupportedOnPlatform = lSymSM("not-supported-on-platform");
}

void lSymbolFree(lSymbol *s){
	 // WIP - should actualle free them in the future
	(void)s;
	/*
	s->nextFree = lSymbolFFree;
	s->c[sizeof(s->c)-1] = 0xFF;
	lSymbolFFree = s;
	lSymbolActive--;
	const int symIndex = lSymIndex(s);
	const int slot = lSymbolBackIndex[symIndex];
	lSymbolIndex[slot] = -symIndex;
	lSymbolMarkMap[s - lSymbolList] = 2;
	*/
}

lSymbol *lSymSL(const char *str, uint len){
	char buf[128];
	len = MIN(sizeof(buf)-1,len);
	memcpy(buf,str,len);
	buf[len] = 0;
	return lSymS(buf);
}

lSymbol *lSymSM(const char *str){
	return lRootsSymbolPush(lSymS(str));
}

lSymbol *lSymS(const char *str){
	lVal oldEntry = lMapRefString(lSymbolTable, str);
	if(oldEntry.type == ltSymbol){
		return (lSymbol *)oldEntry.vSymbol;
	}

	lSymbol *ret;
	if(lSymbolFFree){
		ret = lSymbolFFree;
		lSymbolFFree = lSymbolFFree->nextFree;
	}else{
		if(unlikely(lSymbolMax >= SYM_MAX)){
			exit(112);
		}else{
			ret = &lSymbolList[lSymbolMax++];
		}
	}
	lSymbolActive++;
	ret->hash = lHashString(str, strlen(str));
	strncpy(ret->c, str, sizeof(ret->c));
	ret->c[sizeof(ret->c)-1] = 0;
	lMapSet(lSymbolTable, lValString(str), lValAlloc(ltSymbol, ret));
	return ret;
}

lSymbol *getTypeSymbolT(const lType T){
	switch(T){
		default:            return lSymLTUnknownType;
		case ltNil:         return lSymLTNil;
		case ltBool:        return lSymLTBool;
		case ltPair:        return lSymLTPair;
		case ltEnvironment: return lSymLTEnvironment;
		case ltLambda:      return lSymLTLambda;
		case ltInt:         return lSymLTInt;
		case ltFloat:       return lSymLTFloat;
		case ltString:      return lSymLTString;
		case ltSymbol:      return lSymLTSymbol;
		case ltKeyword:     return lSymLTKeyword;
		case ltNativeFunc:  return lSymLTNativeFunction;
		case ltArray:       return lSymLTArray;
		case ltMacro:       return lSymLTMacro;
		case ltTree:        return lSymLTTree;
		case ltMap:         return lSymLTMap;
		case ltBytecodeArr: return lSymLTBytecodeArray;
		case ltBuffer:      return lSymLTBuffer;
		case ltBufferView:  return lSymLTBufferView;
		case ltFileHandle:  return lSymLTFileHandle;
		case ltAny:         return lSymLTAny;
	}
}

lSymbol *getTypeSymbol(const lVal v){
	return getTypeSymbolT(v.type);
}
