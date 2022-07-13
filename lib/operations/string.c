/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../printer.h"
#include "../type/closure.h"
#include "../type/val.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static lVal *lnfStrlen(lClosure *c, lVal *v){
	return lValInt(lStringLength(requireString(c, lCar(v))));
}

static lVal *lnfTrim(lClosure *c, lVal *v){
	lString *str = requireString(c, lCar(v));

	const char *firstNonWhitespace = str->data;
	const char *bufEnd = &str->data[str->length];
	while(*firstNonWhitespace &&(firstNonWhitespace < (bufEnd-1)) && isspace((u8)*firstNonWhitespace)){
		firstNonWhitespace++;
	}

	const char *lastNonWhitespace = bufEnd;
	while(lastNonWhitespace[-1] && (lastNonWhitespace > (firstNonWhitespace+1)) && isspace((u8)lastNonWhitespace[-1])){
		lastNonWhitespace--;
	}
	lastNonWhitespace = MAX(firstNonWhitespace, MIN(bufEnd, lastNonWhitespace));

	int len = lastNonWhitespace - firstNonWhitespace;
	lVal *ret = lValStringLen(firstNonWhitespace, len);
	return ret;
}

static lVal *lnfStrDown(lClosure *c, lVal *v){
	lString *str = requireString(c, lCar(v));
	const int len = lStringLength(str);

	char *buf = malloc(len+1);
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
	int start, slen, len;
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

lVal *lnfCat(lClosure *c, lVal *v){
	(void)c;
	static char *tmpStringBuf = NULL;
	static int tmpStringBufSize = 1<<12; // Start with 4K
	if(tmpStringBuf == NULL){tmpStringBuf = malloc(tmpStringBufSize);}
	if(tmpStringBuf == NULL){
		fpf(stderr,"lnfCat OOM\n");
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
				fpf(stderr, "lnfCat2 OOM\n");
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
	if(haystack == NULL) {return lValInt(-1);}
	const char *needle   = castToString(lCadr(v),NULL);
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
	return lValSym(requireString(c, lCar(v))->data);
}

static lVal *lnfSymStr(lClosure *c, lVal *v){
	return lValString(requireSymbol(c, lCar(v))->c);
}

static lVal *lnfWriteStr(lClosure *c, lVal *v){
	(void)c;
	char dispWriteBuf[1<<18];
	spf(dispWriteBuf, &dispWriteBuf[sizeof(dispWriteBuf)], "%v", lCar(v));
	return lValString(dispWriteBuf);
}

static lVal *lnfCharAt(lClosure *c,lVal *v){
	const lString *str = requireString(c, lCar(v));
	const int pos = requireInt(c, lCadr(v));
	const int len = lStringLength(str);

	if((pos < 0) || (pos >= len)){
		lExceptionThrowValClo("bounds-error","[char-at] index bigger that string", v, c);
		return NULL;
	}

	return lValInt(str->data[pos]);
}

static lVal *lnfFromCharCode(lClosure *c,lVal *v){
	(void)c;
	int len = lListLength(v)+1;
	char *buf = malloc(len);
	int i=0;

	while(v != NULL){
		buf[i++] = requireInt(c,lCar(v));
		v = lCdr(v);
		if(i >= len){break;}
	}
	buf[i] = 0;

	return lValStringNoCopy(buf, len);
}

void lOperationsString(lClosure *c){
	lAddNativeFuncPure(c,"cat",           "args",                     "ConCATenates ARGS into a single string",                     lnfCat);
	lAddNativeFuncPure(c,"trim",          "[str]",                    "Trim STR of any excessive whitespace",                       lnfTrim);
	lAddNativeFuncPure(c,"string/length", "[str]",                    "Return length of STR",                                       lnfStrlen);
	lAddNativeFuncPure(c,"uppercase",     "[str]",                    "Return STR uppercased",                                      lnfStrUp);
	lAddNativeFuncPure(c,"lowercase",     "[str]",                    "Return STR lowercased",                                      lnfStrDown);
	lAddNativeFuncPure(c,"capitalize",    "[str]",                    "Return STR capitalized",                                     lnfStrCap);
	lAddNativeFuncPure(c,"string/cut",    "[str start &stop]",        "Return STR starting at position START=0 and ending at &STOP=[str-len s]", lnfStringCut);
	lAddNativeFuncPure(c,"index-of",      "[haystack needle &start]", "Return the position of NEEDLE in HAYSTACK, searcing from START=0, or -1 if not found",lnfIndexOf);
	lAddNativeFuncPure(c,"last-index-of", "[haystack needle &start]", "Return the last position of NEEDLE in HAYSTACK, searcing from START=0, or -1 if not found",lnfLastIndexOf);
	lAddNativeFuncPure(c,"char-at",       "[str pos]",                "Return the character at position POS in STR",                lnfCharAt);
	lAddNativeFuncPure(c,"from-char-code","codes",                    "Construct a string out of ...CODE codepoints and return it", lnfFromCharCode);

	lAddNativeFuncPure(c,"string->symbol","[str]",                    "Convert STR to a symbol",                                    lnfStrSym);
	lAddNativeFuncPure(c,"symbol->string","[sym]",                    "Convert SYM to a string",                                    lnfSymStr);

	lAddNativeFuncPure(c,"string/write",     "[val]",                    "Write V into a string and return it",                        lnfWriteStr);
}
