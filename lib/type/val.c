/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "val.h"

#include "../allocation/allocator.h"
#include "../allocation/garbage-collection.h"
#include "../allocation/symbol.h"
#include "../printer.h"
#include "../type/tree.h"
#include "../type/closure.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

lVal *lValInt(i64 v){
	lVal *ret = lValAlloc(ltInt);
	ret->vInt = v;
	return ret;
}

lVal *lValFloat(double v){
	if(unlikely(isnan(v))){
		lExceptionThrowValClo("float-nan","NaN is disallowed in Nujel", NULL, NULL);
	}else if(unlikely(isinf(v))){
		lExceptionThrowValClo("float-inf","INF is disallowed in Nujel", NULL, NULL);
	}
	lVal *ret   = lValAlloc(ltFloat);
	ret->vFloat = v;
	return ret;
}

lVal *lValVec(const vec v){
	lVal *ret = lValAlloc(ltVec);
	ret->vVec = v;
	return ret;
}

lVal *lValBool(bool v){
	lVal *ret = lValAlloc(ltBool);
	ret->vBool = v;
	return ret;
}

lVal *lValTree(lTree *v){
	lVal *ret = lValAlloc(ltTree);
	ret->vTree = v ? v : lTreeNew(NULL, NULL);
	return ret;
}

lVal *lValObject(lClosure *v){
	lVal *ret = lValAlloc(ltObject);
	ret->vClosure = v;
	return ret;
}

lVal *lValLambda(lClosure *v){
	lVal *ret = lValAlloc(ltLambda);
	ret->vClosure = v;
	return ret;
}

/* Checks if A is greater than B, returns 0 if the two values can't be compared
 | or if they are equal.
 */
i64 lValGreater(const lVal *a, const lVal *b){
	if((a == NULL) || (b == NULL)){return 0;}
	if(a->type != b->type){
		if(((a->type == ltInt) || (a->type == ltFloat)) && ((b->type == ltInt) || (b->type == ltFloat))){
			return ((a->type == ltInt) ? (float)a->vInt : a->vFloat) < ((b->type == ltInt) ? (float)b->vInt : b->vFloat) ? -1 : 1;
		}else{
			return 0;
		}
	}
	switch(a->type){
	default:
		return 0;
	case ltKeyword:
	case ltSymbol: {
		const uint alen = strnlen(a->vSymbol->c, sizeof(a->vSymbol->c));
		const uint blen = strnlen(b->vSymbol->c, sizeof(b->vSymbol->c));
		const uint len = MIN(alen,blen);
		const char *ab = a->vSymbol->c;
		const char *bb = b->vSymbol->c;
		for(uint i=0;i<len;i++){
			const u8 ac = *ab++;
			const u8 bc = *bb++;
			if(ac != bc){
				return ac - bc;
			}
		}
		return alen - blen;
	}

	case ltInt:
		return a->vInt - b->vInt;
	case ltFloat:
		return a->vFloat < b->vFloat ? -1 : 1;
	case ltString: {
		const uint alen = lStringLength(a->vString);
		const uint blen = lStringLength(b->vString);
		const uint len = MIN(alen,blen);
		const char *ab = a->vString->data;
		const char *bb = b->vString->data;
		for(uint i=0;i<len;i++){
			const u8 ac = *ab++;
			const u8 bc = *bb++;
			if(ac != bc){
				return ac - bc;
			}
		}
		return alen - blen;
	}}
}

/* Check two values for equality */
bool lValEqual(const lVal *a, const lVal *b){
	if((a == NULL) || (b == NULL)){
		return ((a == NULL) && (b == NULL));
	}
	if(a->type != b->type){
		if(((a->type == ltInt) || (a->type == ltFloat)) && ((b->type == ltInt) || (b->type == ltFloat))){
			return ((a->type == ltInt) ? (float)a->vInt : a->vFloat) == ((b->type == ltInt) ? (float)b->vInt : b->vFloat);
		}else{
			return false;
		}
	}
	switch(a->type){
	default:
		return false;
	case ltPair:
		return (a->vList.car == b->vList.car) && (a->vList.cdr == b->vList.cdr);
	case ltArray:
		return a->vArray == b->vArray;
	case ltTree:
		return a->vTree == b->vTree;
	case ltKeyword:
	case ltSymbol:
		return b->vSymbol == a->vSymbol;
	case ltObject:
	case ltMacro:
	case ltLambda:
		return b->vClosure == a->vClosure;
	case ltNativeFunc:
		return b->vNFunc == a->vNFunc;
	case ltBytecodeOp:
		return a->vBytecodeOp == b->vBytecodeOp;
	case ltBuffer:
		return a->vBuffer == b->vBuffer;
	case ltBufferView:
		return a->vBufferView == b->vBufferView;
	case ltBool:
		return a->vBool == b->vBool;
        case ltGUIWidget:
		return b->vPointer == a->vPointer;
	case ltInt:
		return a->vInt == b->vInt;
	case ltFloat:
		return a->vFloat == b->vFloat;
	case ltString: {
		const uint alen = lStringLength(a->vString);
		const uint blen = lStringLength(b->vString);
		return (alen == blen) && (strncmp(a->vString->data, b->vString->data, alen) == 0);
	}}
}

/* Create a new string containing a direct reference to STR, STR will be
 * freed by the GC if it ever goes out of scope */
static lString *lStringNewNoCopy(const char *str, uint len){
	if(unlikely(str == NULL)){return NULL;}
	lString *s = lBufferAlloc(len, true);
	s->data    = str;
	return s;
}

/* Create a new string containing a copy of STR[0] - STR[LEN] */
lString *lStringNew(const char *str, uint len){
	if(unlikely(str == NULL)){return NULL;}
	char *nbuf = malloc(len+1);
	if(unlikely(nbuf == NULL)){
		fpf(stderr,"lStringNew OOM");
		exit(2);
	}
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	return lStringNewNoCopy(nbuf, len);
}

/* Return a duplicate of OS */
lString *lStringDup(lString *os){
	return lStringNew(os->data, os->length);
}

/* Return the length of the String S */
int lStringLength(const lString *s){
	return s->length;
}

/* Create a new string value out of S */
lVal *lValStringLen(const char *c, int len){
	if(unlikely(c == NULL)){return NULL;}
	lVal *t = lValAlloc(ltString);
	t->vString = lStringNew(c,len);
	return t->vString == NULL ? NULL : t;
}

/* Create a new string value out of S */
lVal *lValString(const char *c){
	return lValStringLen(c, c == NULL ? 0 : strlen(c));
}

/* Create a new string value out of S, using C directly, which will be
 * freed once the value leaves scope  */
lVal *lValStringNoCopy(const char *c,int len){
	if(unlikely(c == NULL)){return NULL;}
	lVal *t = lValAlloc(ltString);
	t->vString = lStringNewNoCopy(c,len);
	return t;
}

/* Return a string value, containing a mark from ERR to ERREND,
 * which has to be within BUF and BUFSTART */
lVal *lValStringError(const char *bufStart, const char *bufEnd, const char *errStart, const char *err, const char *errEnd){
	const char *lineStart, *lineEnd;
	if(bufEnd <= bufStart){return NULL;}
	if(errStart > err){return NULL;}
	if(errEnd < err){return NULL;}

	for(lineStart = errStart; (lineStart > bufStart) && (*lineStart != '\n'); lineStart--){}
	if(*lineStart == '\n'){lineStart++;}
	for(lineEnd = errEnd; (lineEnd < bufEnd) && (*lineEnd != '\n'); lineEnd++){}

	const char *msgStart = MAX(lineStart, (errStart - 30));
	const char *msgEnd   = MIN(lineEnd, (errEnd + 30));
	const size_t bufSize = (msgEnd - msgStart) + 3 + 3 + 1;
	char *outbuf    = malloc(bufSize);
	char *outbufEnd = &outbuf[bufSize];
	char *data      = outbuf;

	if(msgStart != lineStart){data = spf(data, outbufEnd, "...");}
	memcpy(data, msgStart, msgEnd - msgStart);
	data += (msgEnd - msgStart);
	if(msgEnd != lineEnd){data = spf(data, outbufEnd, "...");}
	*data = 0;
	return lValStringNoCopy(outbuf, data - outbuf);
}

/* Return a newly allocated nujel symbol of value S */
lVal *lValSymS(const lSymbol *s){
	if(unlikely(s == NULL)){return NULL;}
	lVal *ret = lValAlloc(ltSymbol);
	ret->vSymbol = s;
	return ret;
}

/* Return a nujel value for the symbol within S */
lVal *lValSym(const char *s){
	return lValSymS(lSymS(s));
}

/* Return a newly allocated nujel keyword of value S */
lVal *lValKeywordS(const lSymbol *s){
	if(unlikely(s == NULL)){return NULL;}
	lVal *ret = lValAlloc(ltKeyword);
	ret->vSymbol = s;
	return ret;
}

/* Return a nujel value for the keyword within S */
lVal *lValKeyword(const char *s){
	return lValKeywordS(lSymS(s));
}

lVal *lValBufferNoCopy(void *data, size_t length, bool immutable){
	lVal *ret = lValAlloc(ltBuffer);
	lBuffer *buf = lBufferAllocRaw();
	buf->buf = data;
	buf->length = length;
	buf->flags = immutable ? BUFFER_IMMUTABLE : 0;
	ret->vBuffer = buf;
	return ret;
}
