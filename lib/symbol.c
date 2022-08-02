/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <stdlib.h>
#include <string.h>

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
int      lSymbolIndex[SYM_MAX];
int      lSymbolBackIndex[SYM_MAX];
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

lSymbol *symType;
lSymbol *symArguments;
lSymbol *symCode;
lSymbol *symData;

lSymbol *lSymLTNil;
lSymbol *lSymLTNoAlloc;
lSymbol *lSymLTBool;
lSymbol *lSymLTPair;
lSymbol *lSymLTLambda;
lSymbol *lSymLTInt;
lSymbol *lSymLTFloat;
lSymbol *lSymLTString;
lSymbol *lSymLTSymbol;
lSymbol *lSymLTKeyword;
lSymbol *lSymLTNativeFunction;
lSymbol *lSymLTObject;
lSymbol *lSymLTMacro;
lSymbol *lSymLTArray;
lSymbol *lSymLTTree;
lSymbol *lSymLTBytecodeOp;
lSymbol *lSymLTBytecodeArray;
lSymbol *lSymLTBuffer;
lSymbol *lSymLTBufferView;
lSymbol *lSymLTUnknownType;

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

	symType              = lSymSM("type");
	symArguments         = lSymSM("arguments");
	symCode              = lSymSM("code");
	symData              = lSymSM("data");

	lSymLTNil            = lSymSM("nil");
	lSymLTNoAlloc        = lSymSM("no-alloc");
	lSymLTBool           = lSymSM("bool");
	lSymLTPair           = lSymSM("pair");
	lSymLTObject         = lSymSM("object");
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
	lSymLTBytecodeOp     = lSymSM("bytecode-op");
	lSymLTBytecodeArray  = lSymSM("bytecode-array");
	lSymLTBuffer         = lSymSM("buffer");
	lSymLTBufferView     = lSymSM("buffer-view");
	lSymLTUnknownType    = lSymSM("unknown-type");
}

void lSymbolFree(lSymbol *s){
	s->nextFree = lSymbolFFree;
	s->c[sizeof(s->c)-1] = 0xFF;
	lSymbolFFree = s;
	lSymbolActive--;
	int symIndex = lSymIndex(s);
	int slot = lSymbolBackIndex[symIndex];
	lSymbolIndex[slot] = -symIndex;
}

lSymbol *lSymSL(const char *str, uint len){
	char buf[32];
	len = MIN(sizeof(buf)-1,len);
	memcpy(buf,str,len);
	buf[len] = 0;
	return lSymS(buf);
}

lSymbol *lSymSM(const char *str){
	return lRootsSymbolPush(lSymS(str));
}

const u32 hashLookupTable[8] = {
	0x2e003dc5,
	0x8b27c03c,
	0x4d9b3063,
	0xbd3e8d7e,
	0x52568b75,
	0x1d528623,
	0xf0a5bd1d,
	0x76c15bf8
};

static u32 lHashSymStr(const char *str){
	u32 res = 0xf3b06b3b;
	while (*str) {
		res = (res << 4) | ((res & 0xF0000000) >> 28);
		res ^= *str++;
#ifdef _MSC_VER
		res ^= hashLookupTable[__popcnt(res) & 0x7];
#else
		res ^= hashLookupTable[__builtin_popcount(res)&0x7];
#endif
	}
	return res;
}

// Probes the symbol index and returns the slot where STR is stored.  If STR is
// not in the map, returns a slot where it could be inserted, which could be
// either an empty slot or a tomb slot from when a different symbol with a hash
// collision was deleted.
uint lSymbolIndexSlot(const char *str){
	uint firstTomb = 0xffffffff;
	uint h = lHashSymStr(str) % SYM_MAX;
	uint hInitial = h;
	symbolLookups++;
	do {
		tombLookups++;
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
	return ret;
}

lSymbol *getTypeSymbolT(const lType T){
	switch(T){
		default:            return lSymLTUnknownType;
		case ltNoAlloc:     return lSymLTNoAlloc;
		case ltBool:        return lSymLTBool;
		case ltPair:        return lSymLTPair;
		case ltObject:      return lSymLTObject;
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
		case ltBytecodeOp:  return lSymLTBytecodeOp;
		case ltBytecodeArr: return lSymLTBytecodeArray;
		case ltBuffer:      return lSymLTBuffer;
		case ltBufferView:  return lSymLTBufferView;
	}
}

lSymbol *getTypeSymbol(const lVal *v){
	return likely(v) ? getTypeSymbolT(v->type) : lSymLTNil;
}
