/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "string.h"
#include "../nujel.h"
#include "../datatypes/list.h"
#include "../datatypes/native-function.h"
#include "../datatypes/string.h"
#include "../datatypes/symbol.h"
#include "../datatypes/val.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lVal *lnfStrlen(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return lValInt(0);}
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return lValInt(0);}
	if(lStrNull(t)){return lValInt(0);}
	return lValInt(lStringLength(&lStr(t)));
}

lVal *lnfTrim(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return NULL;}
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	if(lStrNull(t)){return lValInt(0);}
	const char *s;
	for(s = lStrData(t);*s != 0 && isspace((u8)*s);s++){}
	int len = lStringLength(&lStr(t)) - (s -  lStrData(t));
	for(;len > 0 && isspace((u8)s[len-1]);len--){}
	char *buf = malloc(len+1);
	memcpy(buf,s,len);
	buf[len] = 0;
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vCdr = lStringAlloc();
	if(ret->vCdr == 0){return NULL;}
	lStr(ret).flags |= lfHeapAlloc;
	lStr(ret).buf = lStr(ret).data = buf;
	lStr(ret).bufEnd = &lStrBuf(ret)[len];
	return ret;
}

lVal *lnfStrDown(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return NULL;}
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	if(lStrNull(t)){return lValInt(0);}
	const int len = lStringLength(&lStr(t));
	char *buf = malloc(len+1);
	for(int i=0;i<len;i++){
		buf[i] = tolower((u8)lStrData(t)[i]);
	}
	buf[len] = 0;
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vCdr = lStringAlloc();
	if(ret->vCdr == 0){return NULL;}
	lStr(ret).flags |= lfHeapAlloc;
	lStr(ret).buf = lStr(ret).data = buf;
	lStr(ret).bufEnd = &lStrBuf(ret)[len];
	return ret;
}

lVal *lnfStrUp(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return NULL;}
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	if(lStrNull(t)){return lValInt(0);}
	const int len = lStringLength(&lStr(t));
	char *buf = malloc(len+1);
	for(int i=0;i<len;i++){
		buf[i] = toupper((u8)lStrData(t)[i]);
	}
	buf[len] = 0;
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vCdr = lStringAlloc();
	if(ret->vCdr == 0){return NULL;}
	lStrFlags(ret) |= lfHeapAlloc;
	lStrBuf(ret) = lStrData(ret) = buf;
	lStrEnd(ret) = &lStrBuf(ret)[len];
	return ret;
}

lVal *lnfStrCap(lClosure *c, lVal *v){
	(void)c;
	if(v == NULL){return NULL;}
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	if(lStrNull(t)){return lValInt(0);}
	const int len = lStringLength(&lStr(t));
	char *buf = malloc(len+1);
	int cap = 1;
	for(int i=0;i<len;i++){
		if(isspace((u8)lStrData(t)[i])){
			cap = 1;
			buf[i] = lStrData(t)[i];
		}else{
			if(cap){
				buf[i] = toupper((u8)lStrData(t)[i]);
				cap = 0;
			}else{
				buf[i] = tolower((u8)lStrData(t)[i]);
			}
		}
	}
	buf[len] = 0;
	lVal *ret = lValAlloc();
	ret->type = ltString;
	ret->vCdr = lStringAlloc();
	if(ret->vCdr == 0){return NULL;}
	lStrFlags(ret) |= lfHeapAlloc;
	lStrBuf(ret) = lStrData(ret) = buf;
	lStrEnd(ret) = &lStrBuf(ret)[len];
	return ret;
}

lVal *lnfSubstr(lClosure *c, lVal *v){
	(void)c;
	const char *buf;
	int start = 0;
	int len   = 0;
	int slen  = 0;
	if(v == NULL){return NULL;}
	lVal *str = lCar(v);
	if(str == NULL)          {return NULL;}
	if(str->type != ltString){return NULL;}
	if(str->vCdr == 0)       {return NULL;}
	buf  = lStrData(str);
	slen = len = lStringLength(&lStr(str));

	if(lCdr(v) != NULL){
		v = lCdr(v);
		lVal *lStart = lCar(v);
		if((lStart != NULL) && (lStart->type == ltInt)){
			start = lStart->vInt;
		}
		if(lCdr(v) != NULL){
			v = lCdr(v);
			lVal *lLen = lCar(v);
			if((lLen != NULL) && (lLen->type == ltInt)){
				len = lLen->vInt;
			}
		}
	}
	if(start >= slen){return NULL;}
	if(start < 0){start = slen + start;}
	if(len < 0)  {len   = slen + len;}
	len = MIN(slen,len-start);

	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltString;
	ret->vCdr = lStringNew(&buf[start], len);
	return ret;
}

lVal *lnfCat(lClosure *c, lVal *v){
	(void)c;
	char tmpStringBuf[1<<20];
	char *buf = tmpStringBuf;
	forEach(sexpr,v){
		lVal *t = lCar(sexpr);
		int clen = 0;
		if(t == NULL){continue;}
		switch(t->type){
		default: break;
		case ltInf: {
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"#inf");
			break; }
		case ltSymbol: {
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%s",lvSym(t->vCdr)->c);
			break; }
		case ltFloat: {
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%.5f",t->vFloat);
			for(;buf[clen-1] == '0';clen--){buf[clen]=0;}
			if(buf[clen] == '0'){buf[clen] = 0;}
			if(buf[clen-1] == '.'){buf[clen++] = '0';}
			break; }
		case ltInt: {
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%i",t->vInt);
			break; }
		case ltBool: {
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%s",t->vBool ? "#t" : "#f");
			break; }
		case ltString:
			if(t->vCdr == 0){continue;}
			clen = snprintf(buf,sizeof(tmpStringBuf) - (buf-tmpStringBuf),"%s",lStrData(t));
			break;
		}
		if(clen > 0){
			buf += clen;
		}
	}
	*buf = 0;
	return lValString(tmpStringBuf);
}

lVal *lnfIndexOf(lClosure *c, lVal *v){
	const char *haystack = NULL;
	const char *needle = NULL;

	v = getLArgS(c,v,&haystack);
	if(haystack == NULL) {return lValInt(-1);}
	const int haystackLength = strlen(haystack);

	v = getLArgS(c,v,&needle);
	if(needle == NULL)   {return lValInt(-2);}
	const int needleLength = strlen(needle);

	int pos = 0;
	v = getLArgI(c,v,&pos);
	if(pos > haystackLength-needleLength){return lValInt(-3);}
	/* Empty strings just return the current position, this is so we can
         * split an empty string into each character by passing an empty string
         */
	if(needleLength <= 0){return lValInt(pos);}

	for(const char *s = &haystack[pos]; *s != 0; s++){
		if(strncmp(s,needle,needleLength) == 0){
			return lValInt(s-haystack);
		}
	}
	return lValInt(-4);
}

lVal *lnfLastIndexOf(lClosure *c, lVal *v){
	const char *haystack = NULL;
	const char *needle = NULL;

	v = getLArgS(c,v,&haystack);
	if(haystack == NULL) {return lValInt(-1);}
	const int haystackLength = strlen(haystack);

	v = getLArgS(c,v,&needle);
	if(needle == NULL)   {return lValInt(-2);}
	const int needleLength = strlen(needle);

	if(needleLength <= 0){return lValInt(-3);}
	int pos = haystackLength - needleLength - 1;
	v = getLArgI(c,v,&pos);

	for(const char *s = &haystack[pos]; s > haystack; s--){
		if(strncmp(s,needle,needleLength) == 0){
			return lValInt(s-haystack);
		}
	}
	return lValInt(-4);
}

lVal *lnfStrSym(lClosure *c, lVal *v){
	(void)c;
	v = lCar(v);
	if(v == NULL){return NULL;}
	if(v->type != ltString){return NULL;}
	return lValSym(lStrData(v));
}

lVal *lnfSymStr(lClosure *c, lVal *v){
	(void)c;
	v = lCar(v);
	if(v == NULL){return NULL;}
	if(v->type != ltSymbol){return NULL;}
	return lValString(lvSym(v->vCdr)->c);
}

lVal *lnfWriteStr(lClosure *c, lVal *v){
	(void)c;
	static char *buf = NULL;
	if(v == NULL){
		return lValString("#nil");
	}
	if(buf == NULL){buf = malloc(1<<16);}
	lSWriteVal(lCar(v), buf, &buf[1<<16],0,false);
	buf[(1<<16)-1]=0;
	return lValString(buf);
}

lVal *lnfCharAt(lClosure *c,lVal *v){
	const char *str = NULL;
	int pos = 0;

	v = getLArgS(c,v,&str);
	v = getLArgI(c,v,&pos);

	if(str == NULL){return NULL;}
	const int len = strlen(str);
	if(pos >= len){return NULL;}
	return lValInt(str[pos]);
}

lVal *lnfFromCharCode(lClosure *c,lVal *v){
	int len = lListLength(v)+1;
	char *buf = malloc(len);
	int i=0,code=0;

	while(v != NULL){
		v = getLArgI(c,v,&code);
		buf[i++] = code;
		if(i >= len){break;}
	}
	v = lValString(buf);
	free(buf);
	return v;
}

void lOperationsString(lClosure *c){
	lAddNativeFunc(c,"str/concatenate str/cat cat","[...args]",       "ConCATenates ARGS into a single string",                                                     lnfCat);
	lAddNativeFunc(c,"str/trim trim",              "[str]",           "Trim STR of any excessive whitespace",                                                         lnfTrim);
	lAddNativeFunc(c,"str/length",                 "[str]",      "Return length of STR",                                                                  lnfStrlen);
	lAddNativeFunc(c,"str/uppercase uppercase",    "[str]",           "Return STR uppercased",                                                   lnfStrUp);
	lAddNativeFunc(c,"str/lowercase lowercase",    "[str]",           "Return STR lowercased",                                                   lnfStrDown);
	lAddNativeFunc(c,"str/capitalize capitalize",  "[str]",           "Return STR capitalized",                                                      lnfStrCap);
	lAddNativeFunc(c,"str/substr substr",          "[str &start &stop]","Return STR starting at position START=0 and ending at &STOP=[str-len s]",  lnfSubstr);
	lAddNativeFunc(c,"str/index-of index-of",      "[haystack needle &start]","Return the position of NEEDLE in HAYSTACK, searcing from START=0, or -1 if not found",lnfIndexOf);
	lAddNativeFunc(c,"str/last-index-of",          "[haystack needle &start]","Return the last position of NEEDLE in HAYSTACK, searcing from START=0, or -1 if not found",lnfLastIndexOf);
	lAddNativeFunc(c,"str/char-at char-at",        "[str pos]",       "Return the character at position POS in STR",                                                  lnfCharAt);
	lAddNativeFunc(c,"str/from-char-code from-char-code","[...codes]",      "Construct a string out of ...CODE codepoints and return it",                                    lnfFromCharCode);
	lAddNativeFunc(c,"str->sym",      "[str]",           "Convert STR to a symbol",                                                                      lnfStrSym);
	lAddNativeFunc(c,"sym->str",      "[sym]",           "Convert SYM to a string",                                                                      lnfSymStr);
	lAddNativeFunc(c,"str/write",     "[val]",           "Write V into a string and return it",                                                         lnfWriteStr);
}
