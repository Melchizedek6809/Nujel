/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include "../third-party/fasthash/fasthash.h"

lSymbol  lSymbolList[SYM_MAX];
uint     lSymbolActive = 0;
uint     lSymbolMax    = 0;
lSymbol *lSymbolFFree = NULL;

// a hash index over the symbol array.  It is essentially a sparse map such that
// for a string S, lHashSymStr(S) is a slot in the hash table (with linear
// probing if there's a hash collision).  Then lSymbolIndex[slot] is the
// position in lSymbolList where the actual symbol is stored, and
// lSymbolBackIndex[pos] is the opposite mapping from a position in lSymbolList
// to the corresponding slot in the hash table.
// lSymbolIndex stores all indices 1-based, because it allows us to use 0 to
// mean that the slot is empty which is more convenient.  negative values denote
// deleted slots, and positive values denote used slots.
i16      lSymbolIndex[SYM_MAX];
i16      lSymbolBackIndex[SYM_MAX];
#define SYMBOL_SLOT_IS_EMPTY(X) (X == 0)
#define SYMBOL_SLOT_IS_TOMB(X)  (X < 0)
#define SYMBOL_SLOT_IS_USED(X)  (X > 0)

lSymbol *symNull;
lSymbol *symQuote;
lSymbol *symQuasiquote;
lSymbol *symUnquote;
lSymbol *symUnquoteSplicing;
lSymbol *symArr;
lSymbol *symTreeNew;
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

uint symbolLookups = 0;
uint tombLookups = 0;

void lSymbolInit(){
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

static inline int lSymIndex(const lSymbol *s){
	return s - lSymbolList;
}

void lSymbolFree(lSymbol *s){
	s->nextFree = lSymbolFFree;
	s->c[sizeof(s->c)-1] = 0xFF;
	lSymbolFFree = s;
	lSymbolActive--;
	const int symIndex = lSymIndex(s);
	const int slot = lSymbolBackIndex[symIndex];
	lSymbolIndex[slot] = -symIndex;
	lSymbolMarkMap[s - lSymbolList] = 2;
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

static inline u32 lHashSymStr(const char *str){
	return fasthash32(str, strlen(str), 0x5b0a159d9eac0381ULL);
}

// Probes the symbol index and returns the slot where STR is stored.  If STR is
// not in the map, returns a slot where it could be inserted, which could be
// either an empty slot or a tomb slot from when a different symbol with a hash
// collision was deleted.
uint lSymbolIndexSlot(const char *str){
	uint firstTomb = 0xffffffff;
	uint h = lHashSymStr(str) % SYM_MAX;
	uint hInitial = h;
	//symbolLookups++;
	do {
		//tombLookups++;
		int idx = lSymbolIndex[h];
		if(SYMBOL_SLOT_IS_EMPTY(idx)){
			return firstTomb == 0xffffffff ? h : firstTomb;
		}
		if(SYMBOL_SLOT_IS_TOMB(idx) && firstTomb == 0xffffffff){
			firstTomb = h;
		}
		if(SYMBOL_SLOT_IS_USED(idx)){
			--idx;
			if (0 == strncmp(str,lSymbolList[idx].c,sizeof(lSymbolList[idx].c)-1)
				&& 0 == lSymbolList[idx].c[sizeof(lSymbolList[idx].c)-1])
			{
				return h;
			}
		}
		if (++h == SYM_MAX) {
			h = 0;
		}
	} while (h != hInitial);
	exit(111);
	return 0;
}

lSymbol *lSymS(const char *str){
	uint slot = lSymbolIndexSlot(str);
	int symIndex = lSymbolIndex[slot];
	if(SYMBOL_SLOT_IS_USED(symIndex)){
		return &lSymbolList[symIndex-1];
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
	strncpy(ret->c, str, sizeof(ret->c));
	ret->c[sizeof(ret->c)-1] = 0;
	symIndex = lSymIndex(ret);
	lSymbolIndex[slot] = symIndex + 1;
	lSymbolBackIndex[symIndex] = slot;
	lSymbolMarkMap[ret - lSymbolList] = 0;
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
