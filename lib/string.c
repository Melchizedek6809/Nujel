/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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
	char *nbuf = malloc(((i64)len)+1);
	if(unlikely(nbuf == NULL)){
		exit(23);
	}
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	return lStringNewNoCopy(nbuf, len);
}

/* Return a duplicate of OS */
lString *lStringDup(const lString *os){
	return lStringNew(os->data, os->length);
}

/* Create a new string value out of S */
lVal lValStringLen(const char *c, int len){
	if(unlikely(c == NULL)){return NIL;}
	lVal t = lValAlloc(ltString, lStringNew(c,len));
	return unlikely(t.vString == NULL) ? NIL : t;
}

/* Create a new string value out of S */
lVal lValString(const char *c){
	return lValStringLen(c, unlikely(c == NULL) ? 0 : strlen(c));
}

/* Create a new string value out of S, using C directly, which will be
 * freed once the value leaves scope  */
lVal lValStringNoCopy(const char *c,int len){
	if(unlikely(c == NULL)){return NIL;}
	return lValAlloc(ltString, lStringNewNoCopy(c,len));
}

/* Return a string value, containing a mark from ERR to ERREND,
 * which has to be within BUF and BUFSTART */
lVal lValStringError(const char *bufStart, const char *bufEnd, const char *errStart, const char *err, const char *errEnd){
	const char *lineStart, *lineEnd;
	if(bufEnd <= bufStart){return NIL;}
	if(errStart > err){return NIL;}
	if(errEnd < err){return NIL;}

	for(lineStart = errStart; (lineStart > bufStart) && (*lineStart != '\n'); lineStart--){}
	if(*lineStart == '\n'){lineStart++;}
	for(lineEnd = errEnd; (lineEnd < bufEnd) && (*lineEnd != '\n'); lineEnd++){}
	lineEnd = MAX(lineEnd, lineStart);

	const char *msgStart = MAX(lineStart, (errStart - 30));
	const char *msgEnd   = MAX(msgStart, MIN(lineEnd, (errEnd + 30)));
	const size_t bufSize = (msgEnd - msgStart) + 3 + 3 + 1;
	char *outbuf    = malloc(bufSize);
	if (unlikely(outbuf == NULL)) {
		return NIL;
	}
	char *data = outbuf;

	memcpy(data, msgStart, msgEnd - msgStart);
	data += (msgEnd - msgStart);
	*data = 0;
	return lValStringNoCopy(outbuf, data - outbuf);
}

static lVal lnmStringCut(lVal self, lVal start, lVal stop){
	i64 slen, len;
	const char *buf = self.vString->data;
	slen = len = lBufferLength(self.vString);
	reqInt(start);
	i64 off = MAX(0, start.vInt);
	len = MIN(slen - off, (((stop.type == ltInt)) ? stop.vInt : len) - off);

	if(unlikely(len <= 0)){return lValString("");}
	return lValStringLen(&buf[off], len);
}

static lVal lnmStringIndexOf(lVal self, lVal search, lVal start){
	const char *haystack = self.vString->data;
	if(unlikely(search.type != ltString)){
		return lValExceptionType(search, ltString);
	}
	const char *needle = search.vString->data;
	const i64 haystackLength = self.vString->length;
	const i64 needleLength   = search.vString->length;

	if(needleLength <= 0){return lValInt(-1);}
	const i64 pos = castToInt(start, 0);
	if(pos > haystackLength-needleLength){return lValInt(-3);}
	/* Empty strings just return the current position, this is so we can
	 * split an empty string into each character by passing an empty string
	 */
	if(needleLength <= 0){return lValInt(pos);}

	for(const char *s = &haystack[pos]; *s != 0; s++){
		if(strncmp(s,needle,needleLength)){continue;}
		return lValInt(s-haystack);
	}
	return lValInt(-1);
}

static lVal lnmStringLastIndexOf(lVal self, lVal search, lVal start){
	const char *haystack = self.vString->data;
	if(unlikely(search.type != ltString)){
		return lValExceptionType(search, ltString);
	}
	const char *needle = search.vString->data;
	const i64 haystackLength = self.vString->length;
	const i64 needleLength   = search.vString->length;

	if(needleLength <= 0){return lValInt(-1);}
	const i64 pos = castToInt(start, haystackLength - needleLength);

	for(const char *s = &haystack[pos]; s >= haystack; s--){
		if(strncmp(s,needle,needleLength)){continue;}
		return lValInt(s-haystack);
	}
	return lValInt(-1);
}

void lOperationsString(){
	lClass *String = &lClassList[ltString];
	lAddNativeMethodVVV(String, lSymS("cut"),      "(self start stop)", lnmStringCut, NFUNC_PURE);
	lAddNativeMethodVVV(String, lSymS("index-of"), "(self search start)", lnmStringIndexOf, NFUNC_PURE);
	lAddNativeMethodVVV(String, lSymS("last-index-of"), "(self search start)", lnmStringLastIndexOf, NFUNC_PURE);
}
