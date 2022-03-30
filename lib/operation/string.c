/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../display.h"
#include "../exception.h"
#include "../collection/string.h"
#include "../misc/pf.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static lVal *lnfStrlen(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){
		lExceptionThrowValClo("type-error","[string/length] expects a string as its first and only argument", v, c);
		return NULL;
	}
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){
		lExceptionThrowValClo("type-error","[string/length] expects a string as its first and only argument", v, c);
		return NULL;
	}
	return lValInt(lStringLength(t->vString));
}

static lVal *lnfTrim(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return NULL;}
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	const char *s;
	for(s = t->vString->data;*s != 0 && isspace((u8)*s);s++){}
	int len = lStringLength(t->vString) - (s -  t->vString->data);
	for(;len > 0 && isspace((u8)s[len-1]);len--){}
	char *buf = malloc(len+1);
	memcpy(buf,s,len);
	buf[len] = 0;
	lVal *ret = lValStringLen(buf, len);
	free(buf);
	return ret;
}

static lVal *lnfStrDown(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return NULL;}
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	const int len = lStringLength(t->vString);
	char *buf = malloc(len+1);
	for(int i=0;i<len;i++){
		buf[i] = tolower((u8)t->vString->data[i]);
	}
	buf[len] = 0;
	lVal *ret = lValStringLen(buf, len);
	free(buf);
	return ret;
}

static lVal *lnfStrUp(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return NULL;}
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	const int len = lStringLength(t->vString);
	char *buf = malloc(len+1);
	for(int i=0;i<len;i++){
		buf[i] = toupper((u8)t->vString->data[i]);
	}
	buf[len] = 0;
	lVal *ret = lValStringLen(buf, len);
	free(buf);
	return ret;
}

static lVal *lnfStrCap(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return NULL;}
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	const int len = lStringLength(t->vString);
	char *buf = malloc(len+1);
	int cap = 1;
	for(int i=0;i<len;i++){
		if(isspace((u8)t->vString->data[i])){
			cap = 1;
			buf[i] = t->vString->data[i];
		}else{
			if(cap){
				buf[i] = toupper((u8)t->vString->data[i]);
				cap = 0;
			}else{
				buf[i] = tolower((u8)t->vString->data[i]);
			}
		}
	}
	buf[len] = 0;
	lVal *ret = lValStringLen(buf, len);
	free(buf);
	return ret;
}

static lVal *lnfStringCut(lClosure *c, lVal *v){
	(void)c;
	int start, slen, len;
	lVal *str = lCar(v);
	if((str == NULL) || (str->type != ltString)){
		lExceptionThrowValClo("type-error","[string/cut] expects a string as its first and only argument", v, c);
		return NULL;
	}

	const char *buf = str->vString->data;
	slen = len = lStringLength(str->vString);
	start = MAX(0, castToInt(lCadr(v), 0));
	len   = MIN(slen - start, castToInt(lCaddr(v), len) - start);

	if(len <= 0){return lValString("");}
	return lValStringLen(&buf[start], len);
}

lVal *lnfCat(lClosure *c, lVal *v){
	(void)c;
	static char *tmpStringBuf = NULL;
	static int tmpStringBufSize = 1<<12; // Start with 4K
	if(tmpStringBuf == NULL){tmpStringBuf = malloc(tmpStringBufSize);}
	if(tmpStringBuf == NULL){
		lPrintError("lnfCat OOM\n");
		return NULL;
	}
	char *new, *cur = tmpStringBuf;
	char *bufEnd = &tmpStringBuf[tmpStringBufSize];
	for(lVal *sexpr = v; sexpr; sexpr = sexpr->vList.cdr){
		lVal *car;
		restart:
		car = sexpr;
		if(car->type == ltPair){car = sexpr->vList.car;}
		if(car == NULL){continue;}
		new = spf(cur, bufEnd, "%V", car);
		if(new >= (bufEnd-1)){ // Doesn't Work right now!!!!!
			tmpStringBufSize *= 2;
			const int i = cur - tmpStringBuf;
			tmpStringBuf = realloc(tmpStringBuf,tmpStringBufSize);
			bufEnd = &tmpStringBuf[tmpStringBufSize];
			if(tmpStringBuf == NULL){
				lPrintError("lnfCat2 OOM\n");
				return NULL;
			}
			cur = &tmpStringBuf[i];
			goto restart;
		}
		cur = new;
		if(sexpr->type != ltPair){break;}
	}
	if(cur < bufEnd){*cur = 0;}
	return lValString(tmpStringBuf);
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
	const char *needle   = castToString(lCadr(v),NULL);
	if(haystack == NULL) {return lValInt(-1);}
	if(needle   == NULL) {return lValInt(-2);}
	const int haystackLength = strlen(haystack);
	const int needleLength   = strlen(needle);

	if(needleLength <= 0){return lValInt(-3);}
	const int pos = castToInt(lCaddr(v),haystackLength - needleLength - 1);

	for(const char *s = &haystack[pos]; s > haystack; s--){
		if(strncmp(s,needle,needleLength)){continue;}
		return lValInt(s-haystack);
	}
	return lValInt(-4);
}

static lVal *lnfStrSym(lClosure *c, lVal *v){
	(void)c;
	v = lCar(v);
	if((v == NULL) || (v->type != ltString)){
		lExceptionThrowValClo("type-error","[str->sym] expects a string as its first and only argument", v, c);
		return NULL;
	}
	return lValSym(v->vString->data);
}

static lVal *lnfSymStr(lClosure *c, lVal *v){
	(void)c;
	v = lCar(v);
	if((v == NULL) || (v->type != ltSymbol)){
		lExceptionThrowValClo("type-error","[sym->str] expects a string as its first and only argument", v, c);
		return NULL;
	}
	return lValString(v->vSymbol->c);
}

static lVal *lnfWriteStr(lClosure *c, lVal *v){
	(void)c;
	spf(dispWriteBuf, &dispWriteBuf[sizeof(dispWriteBuf)], "%v", lCar(v));
	return lValString(dispWriteBuf);
}

static lVal *lnfCharAt(lClosure *c,lVal *v){
	(void)c;
	const char *str = castToString(lCar(v),NULL);
	if(str == NULL){
		lExceptionThrowValClo("type-error","[char-at] expects a string as its first argument", v, c);
		return NULL;
	}
	const int pos = castToInt(lCadr(v),-1);
	if(pos < 0){
		lExceptionThrowValClo("bounds-error","[char-at] does not support negative indices", v, c);
		return NULL;
	}
	const int len = strlen(str);
	if(pos >= len){
		lExceptionThrowValClo("bounds-error","[char-at] index bigger that string", v, c);
		return NULL;
	}
	return lValInt(str[pos]);
}

static lVal *lnfFromCharCode(lClosure *c,lVal *v){
	(void)c;
	int len = lListLength(v)+1;
	char *buf = malloc(len);
	int i=0;

	while(v != NULL){
		buf[i++] = castToInt(lCar(v),0);
		v = lCdr(v);
		if(i >= len){break;}
	}
	buf[i] = 0;
	v = lValStringNoCopy(buf, len);
	return v;
}

void lOperationsString(lClosure *c){
	lAddNativeFunc(c,"cat",           "args",                "ConCATenates ARGS into a single string",                     lnfCat);
	lAddNativeFunc(c,"trim",          "[str]",                    "Trim STR of any excessive whitespace",                       lnfTrim);
	lAddNativeFunc(c,"string/length", "[str]",                    "Return length of STR",                                       lnfStrlen);
	lAddNativeFunc(c,"uppercase",     "[str]",                    "Return STR uppercased",                                      lnfStrUp);
	lAddNativeFunc(c,"lowercase",     "[str]",                    "Return STR lowercased",                                      lnfStrDown);
	lAddNativeFunc(c,"capitalize",    "[str]",                    "Return STR capitalized",                                     lnfStrCap);
	lAddNativeFunc(c,"string/cut",    "[str &start &stop]",       "Return STR starting at position START=0 and ending at &STOP=[str-len s]", lnfStringCut);
	lAddNativeFunc(c,"index-of",      "[haystack needle &start]", "Return the position of NEEDLE in HAYSTACK, searcing from START=0, or -1 if not found",lnfIndexOf);
	lAddNativeFunc(c,"last-index-of", "[haystack needle &start]", "Return the last position of NEEDLE in HAYSTACK, searcing from START=0, or -1 if not found",lnfLastIndexOf);
	lAddNativeFunc(c,"char-at",       "[str pos]",                "Return the character at position POS in STR",                lnfCharAt);
	lAddNativeFunc(c,"from-char-code","codes",               "Construct a string out of ...CODE codepoints and return it", lnfFromCharCode);

	lAddNativeFunc(c,"str->sym",      "[str]",                    "Convert STR to a symbol",                                    lnfStrSym);
	lAddNativeFunc(c,"sym->str",      "[sym]",                    "Convert SYM to a string",                                    lnfSymStr);

	lAddNativeFunc(c,"str/write",     "[val]",                    "Write V into a string and return it",                        lnfWriteStr);
}
