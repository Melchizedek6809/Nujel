/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "symbol.h"
#include "val.h"
#include "../display.h"
#include "../nujel.h"
#include "../allocation/val.h"
#include "../misc/pf.h"

#include <stdlib.h>
#include <stdio.h>
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

lSymbol *symNull,*symQuote,*symQuasiquote,*symUnquote,*symUnquoteSplicing,*symArr,*symIf,*symCond,*symDo,*symMinus,*symLambda,*symLambdAst,*symTreeNew;
lSymbol *lSymLTNil, *lSymLTNoAlloc, *lSymLTBool, *lSymLTPair, *lSymLTLambda, *lSymLTInt, *lSymLTFloat, *lSymLTVec, *lSymLTString, *lSymLTSymbol, *lSymLTNativeFunction, *lSymLTSpecialForm, *lSymLTArray, *lSymLTGUIWidget, *lSymLTObject, *lSymLTDynamic, *lSymLTMacro, *lSymLTTree, *lSymLTBytecodeOp,*lSymLTBytecodeArray;

void lSymbolInit(){
	lSymbolActive   = 0;
	lSymbolMax      = 0;
	lSymbolFFree    = NULL;

	lSymbolList[0].nextFree = NULL;
	lSymbolList[0].c[sizeof(lSymbolList[0].c) - 1] = 0xFF;
	if(lSymbolList[0].nextFree != NULL){
		fpf(stderr, "Overlapping zero byte and nextFree Pointer in symbol table, exiting immediatly\n");
		exit(123);
	}

	symNull            = RSYMP(lSymS(""));
	symQuote           = RSYMP(lSymS("quote"));
	symArr             = RSYMP(lSymS("array/new"));
	symIf              = RSYMP(lSymS("if"));
	symCond            = RSYMP(lSymS("cond"));
	symDo              = RSYMP(lSymS("do"));
	symMinus           = RSYMP(lSymS("-"));
	symLambda          = RSYMP(lSymS("λ"));
	symLambdAst        = RSYMP(lSymS("λ*"));
	symTreeNew         = RSYMP(lSymS("tree/new"));
	symQuasiquote      = RSYMP(lSymS("quasiquote"));
	symUnquote         = RSYMP(lSymS("unquote"));
	symUnquoteSplicing = RSYMP(lSymS("unquote-splicing"));

	lSymLTNil            = RSYMP(lSymS(":nil"));
	lSymLTNoAlloc        = RSYMP(lSymS(":no-alloc"));
	lSymLTBool           = RSYMP(lSymS(":bool"));
	lSymLTPair           = RSYMP(lSymS(":pair"));
	lSymLTObject         = RSYMP(lSymS(":object"));
	lSymLTLambda         = RSYMP(lSymS(":lambda"));
	lSymLTInt            = RSYMP(lSymS(":int"));
	lSymLTFloat          = RSYMP(lSymS(":float"));
	lSymLTVec            = RSYMP(lSymS(":vec"));
	lSymLTString         = RSYMP(lSymS(":string"));
	lSymLTSymbol         = RSYMP(lSymS(":symbol"));
	lSymLTNativeFunction = RSYMP(lSymS(":native-function"));
	lSymLTSpecialForm    = RSYMP(lSymS(":special-form"));
	lSymLTArray          = RSYMP(lSymS(":array"));
	lSymLTGUIWidget      = RSYMP(lSymS(":gui-widget"));
	lSymLTMacro          = RSYMP(lSymS(":macro"));
	lSymLTTree           = RSYMP(lSymS(":tree"));
	lSymLTBytecodeOp     = RSYMP(lSymS(":bytecode-op"));
	lSymLTBytecodeArray  = RSYMP(lSymS(":bytecode-array"));
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

uint lHashSymStr(const char *str)
{
	uint res = 0x12345678;
	while (*str) {
		res = (res << 4) | ((res & 0xf0000000) >> 28);
		res += *str++;
	}
	return res;
}

// Probes the symbol index and returns the slot where STR is stored.  If STR is
// not in the map, returns a slot where it could be inserted, which could be
// either an empty slot or a tomb slot from when a different symbol with a hash
// collision was deleted.
uint lSymbolIndexSlot(const char *str)
{
	uint firstTomb = 0xffffffff;
	uint h = lHashSymStr(str) % SYM_MAX;
	uint hInitial = h;
	do {
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
	lPrintError("lSymbolIndexSlot Overflow\n");
	exit(123);
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
		getFirstFree:
		ret = lSymbolFFree;
		lSymbolFFree = lSymbolFFree->nextFree;
	}else{
		if(lSymbolMax >= SYM_MAX){
			lGarbageCollect();
			if(lSymbolFFree != NULL){
				goto getFirstFree;
			} else {
				lPrintError("lSym Overflow\n");
				exit(123);
			}
		}else{
			ret = &lSymbolList[lSymbolMax++];
		}
	}
	lSymbolActive++;
	spf(ret->c, &ret->c[sizeof(ret->c)], "%s", str);
	ret->c[sizeof(ret->c)-1] = 0;
	symIndex = lSymIndex(ret);
	lSymbolIndex[slot] = symIndex + 1;
	lSymbolBackIndex[symIndex] = slot;
	return ret;
}

lSymbol *getTypeSymbol(const lVal* v){
	if(v == NULL){return lSymLTNil;}
	switch(v->type){
		default:            return lSymLTNil;
		case ltNoAlloc:     return lSymLTNoAlloc;
		case ltBool:        return lSymLTBool;
		case ltPair:        return lSymLTPair;
		case ltObject:      return lSymLTObject;
		case ltLambda:      return lSymLTLambda;
		case ltInt:         return lSymLTInt;
		case ltFloat:       return lSymLTFloat;
		case ltVec:         return lSymLTVec;
		case ltString:      return lSymLTString;
		case ltSymbol:      return lSymLTSymbol;
		case ltNativeFunc:  return lSymLTNativeFunction;
		case ltSpecialForm: return lSymLTSpecialForm;
		case ltArray:       return lSymLTArray;
		case ltGUIWidget:   return lSymLTGUIWidget;
		case ltMacro:       return lSymLTMacro;
		case ltTree:        return lSymLTTree;
		case ltBytecodeOp:  return lSymLTBytecodeOp;
		case ltBytecodeArr: return lSymLTBytecodeArray;
	}
}
