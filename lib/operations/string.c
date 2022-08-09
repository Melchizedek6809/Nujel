/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "../nujel-private.h"
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
	lineEnd = MAX(lineEnd, lineStart);

	const char *msgStart = MAX(lineStart, (errStart - 30));
	const char *msgEnd   = MAX(msgStart, MIN(lineEnd, (errEnd + 30)));
	const size_t bufSize = (msgEnd - msgStart) + 3 + 3 + 1;
	char *outbuf    = malloc(bufSize);
	if (unlikely(outbuf == NULL)) {
		return NULL;
	}
	char *data      = outbuf;

	memcpy(data, msgStart, msgEnd - msgStart);
	data += (msgEnd - msgStart);
	*data = 0;
	return lValStringNoCopy(outbuf, data - outbuf);
}

static lVal *lnfStrDown(lClosure *c, lVal *v){
	lString *str = requireString(c, lCar(v));
	const int len = lStringLength(str);

	char *buf = malloc(len+1);
	if (unlikely(buf == NULL)) {
		lExceptionThrowValClo("out-of-memory", "Couldn't allocate a buffer for [downcase]", v, c);
		return NULL;
	}
	for(int i=0;i<len;i++){
		buf[i] = tolower((u8)str->data[i]);
	}
	buf[len] = 0;
	return lValStringNoCopy(buf, len);
}

static lVal *lnfStrUp(lClosure *c, lVal *v){
	lString *str = requireString(c, lCar(v));
	const int len = lStringLength(str);

	char *buf = malloc(len+1);
	if (unlikely(buf == NULL)) {
		lExceptionThrowValClo("out-of-memory", "Couldn't allocate a buffer for [upcase]", v, c);
		return NULL;
	}
	for(int i=0;i<len;i++){
		buf[i] = toupper((u8)str->data[i]);
	}
	buf[len] = 0;
	return lValStringNoCopy(buf, len);
}

static lVal *lnfStrCap(lClosure *c, lVal *v){
	lString *str = requireString(c, lCar(v));
	const int len = lStringLength(str);

	char *buf = malloc(len+1);
	if (unlikely(buf == NULL)) {
		lExceptionThrowValClo("out-of-memory", "Couldn't allocate a buffer for [capitalize]", v, c);
		return NULL;
	}
	int cap = 1;
	for(int i=0;i<len;i++){
		if(isspace((u8)str->data[i])){
			cap = 1;
			buf[i] = str->data[i];
		}else{
			if(cap){
				buf[i] = toupper((u8)str->data[i]);
				cap = 0;
			}else{
				buf[i] = tolower((u8)str->data[i]);
			}
		}
	}

	buf[len] = 0;
	return lValStringNoCopy(buf, len);
}

static lVal *lnfStringCut(lClosure *c, lVal *v){
	(void)c;
	i64 start, slen, len;
	lVal *str = lCar(v);
	if((str == NULL) || (str->type != ltString)){
		lExceptionThrowValClo("type-error","[string/cut] expects a string as its first and only argument", v, c);
		return NULL;
	}

	const char *buf = str->vString->data;
	slen = len = lStringLength(str->vString);
	start = MAX(0, requireInt(c, lCadr(v)));
	lVal *lenV = lCaddr(v);
	len   = MIN(slen - start, ((lenV && (lenV->type == ltInt)) ? lenV->vInt : len) - start);

	if(len <= 0){return lValString("");}
	return lValStringLen(&buf[start], len);
}

static lVal *lnfIndexOf(lClosure *c, lVal *v){
	(void)c;
	const char *haystack = castToString(lCar(v),NULL);
	const char *needle   = castToString(lCadr(v),NULL);
	if(haystack == NULL) {return lValInt(-1);}
	if(needle   == NULL) {return lValInt(-2);}
	const int haystackLength = strlen(haystack);
	const int needleLength   = strlen(needle);

	const int pos = castToInt(lCaddr(v),0);
	if(pos > haystackLength-needleLength){return lValInt(-3);}
	/* Empty strings just return the current position, this is so we can
         * split an empty string into each character by passing an empty string
         */
	if(needleLength <= 0){return lValInt(pos);}

	for(const char *s = &haystack[pos]; *s != 0; s++){
		if(strncmp(s,needle,needleLength)){continue;}
		return lValInt(s-haystack);
	}
	return lValInt(-4);
}

static lVal *lnfLastIndexOf(lClosure *c, lVal *v){
	(void)c;
	const char *haystack = castToString(lCar(v),NULL);
	if(haystack == NULL) {return lValInt(-1);}
	const char *needle   = castToString(lCadr(v),NULL);
	if(needle   == NULL) {return lValInt(-2);}
	const i64 haystackLength = strlen(haystack);
	const i64 needleLength   = strlen(needle);

	if(needleLength <= 0){return lValInt(-3);}
	const i64 pos = castToInt(lCaddr(v),haystackLength - needleLength - 1);

	for(const char *s = &haystack[pos]; s > haystack; s--){
		if(strncmp(s,needle,needleLength)){continue;}
		return lValInt(s-haystack);
	}
	return lValInt(-4);
}

static lVal *lnfStrSym(lClosure *c, lVal *v){
	return lValSym(requireString(c, lCar(v))->data);
}

void lOperationsString(lClosure *c){
	lAddNativeFuncPure(c,"uppercase",     "[str]",                    "Return STR uppercased",                                      lnfStrUp);
	lAddNativeFuncPure(c,"lowercase",     "[str]",                    "Return STR lowercased",                                      lnfStrDown);
	lAddNativeFuncPure(c,"capitalize",    "[str]",                    "Return STR capitalized",                                     lnfStrCap);
	lAddNativeFuncPure(c,"string/cut",    "[str start &stop]",        "Return STR starting at position START=0 and ending at &STOP=[str-len s]", lnfStringCut);
	lAddNativeFuncPure(c,"index-of",      "[haystack needle &start]", "Return the position of NEEDLE in HAYSTACK, searcing from START=0, or -1 if not found",lnfIndexOf);
	lAddNativeFuncPure(c,"last-index-of", "[haystack needle &start]", "Return the last position of NEEDLE in HAYSTACK, searcing from START=0, or -1 if not found",lnfLastIndexOf);

	lAddNativeFuncPure(c,"string->symbol","[str]",                    "Convert STR to a symbol",                                    lnfStrSym);
}
