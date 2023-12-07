/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

lVal NIL;

static inline i64 lStringGreater(const lBuffer *a, const lBuffer *b) {
	const uint alen = lBufferLength(a);
	const uint blen = lBufferLength(b);
	const uint len	= MIN(alen,blen);
	const char *ab	= a->data;
	const char *bb	= b->data;
	for(uint i=0;i<len;i++){
		const u8 ac = *ab++;
		const u8 bc = *bb++;
		if(ac != bc){
			return ac - bc;
		}
	}
	return alen - blen;
}

static inline i64 lSymbolGreater(const lSymbol *a, const lSymbol *b) {
	const uint alen = strnlen(a->c, sizeof(a->c));
	const uint blen = strnlen(b->c, sizeof(b->c));
	const uint len	= MIN(alen,blen);
	const char *ab	= a->c;
	const char *bb	= b->c;
	for(uint i=0;i<len;i++){
		const u8 ac = *ab++;
		const u8 bc = *bb++;
		if(ac != bc){
			return ac - bc;
		}
	}
	return alen - blen;
}

/* Checks if A is greater than B, returns 0 if the two values can't be compared
 | or if they are equal.
 */
i64 lValGreater(const lVal a, const lVal b){
	if(unlikely(a.type != b.type)){
		if((a.type == ltInt) && (b.type == ltFloat)){
			return (((float)a.vInt) < b.vFloat)
				? -1
				: (((float)a.vInt) > b.vFloat)
				  ? 1
				  : 0;
		} else if ((a.type == ltFloat) && (b.type == ltInt)) {
			return (a.vFloat < ((float)b.vInt))
				? -1
				: (a.vFloat > ((float)b.vInt))
				  ? 1
				  : 0;
		}
		return 0;
	}
	switch(a.type){
	default:
		return 0;
	case ltInt:
		return a.vInt - b.vInt;
	case ltFloat:
		return a.vFloat < b.vFloat ? -1 : 1;
	case ltKeyword:
	case ltSymbol:
		return lSymbolGreater(a.vSymbol, b.vSymbol);
	case ltString:
		return lStringGreater(a.vString, b.vString);
	}
}

/* Check two values for equality */
bool lValEqual(const lVal a, const lVal b) {
	if (unlikely(a.type != b.type)) {
		if ((a.type == ltInt) && (b.type == ltFloat)) {
			return ((float)a.vInt) == b.vFloat;
		} else if ((a.type == ltFloat) && (b.type == ltInt)) {
			return a.vFloat == ((float)b.vInt);
		}
		return false;
	}
	switch(a.type){
	case(ltString):{
		const uint alen = lBufferLength(a.vString);
		const uint blen = lBufferLength(b.vString);
		return (alen == blen) && (memcmp(a.vString->data, b.vString->data, alen) == 0); }
	case(ltBool):
		return a.vBool == b.vBool;
	case(ltInt):
		return a.vInt == b.vInt;
	case(ltFloat):
		return a.vFloat == b.vFloat;
	default:
		return a.vPointer == b.vPointer;
	}
}

lVal lValException(const lSymbol *symbol, const char *error, lVal v) {
	lVal l = lCons(v, NIL);
	l = lCons(lValString(error),l);
	l = lCons(lValKeywordS(symbol),l);
	l.type = ltException;
	return l;
}
