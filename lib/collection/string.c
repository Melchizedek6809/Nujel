/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "string.h"

#include "list.h"
#include "../display.h"
#include "../nujel.h"
#include "../allocation/array.h"
#include "../allocation/garbage-collection.h"
#include "../allocation/string.h"
#include "../allocation/val.h"
#include "../type/closure.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"
#include "../type-system.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Create a new string containing a copy of STR[0] - STR[LEN] */
lString *lStringNew(const char *str, uint len){
	if(str == NULL){return 0;}
	lString *s = lStringAlloc();
	char *nbuf = malloc(len+1);
	if(nbuf == NULL){
		lPrintError("lStringNew OOM");
	}
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	s->buf    = s->data = nbuf;
	s->flags  = HEAP_ALLOCATED;
	s->bufEnd = &s->buf[len];
	return s;
}

/* Create a new string containing a direct reference to STR, STR will be
 * freed by the GC if it ever goes out of scope */
lString *lStringNewNoCopy(const char *str, uint len){
	if(str == NULL){return 0;}
	lString *s = lStringAlloc();
	s->buf    = s->data = str;
	s->flags  = HEAP_ALLOCATED;
	s->bufEnd = &s->buf[len];
	return s;
}

/* Return a duplicate of OS */
lString *lStringDup(lString *os){
	uint len = os->bufEnd - os->buf;
	const char *str = os->data;
	lString *s = lStringAlloc();
	if(s == NULL){return 0;}
	char *nbuf = malloc(len+1);
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	s->buf    = s->data = nbuf;
	s->flags  = HEAP_ALLOCATED;
	s->bufEnd = &s->buf[len];
	return s;
}

/* Return the length of the String S */
int lStringLength(const lString *s){
	return s->bufEnd - s->buf;
}

/* Create a new string value out of S */
lVal *lValStringLen(const char *c, int len){
	if(c == NULL){return NULL;}
	lVal *t = lRootsValPush(lValAlloc());
	t->type = ltString;
	t->vString = lStringNew(c,len);
	return t->vString == NULL ? NULL : t;
}

/* Create a new string value out of S */
lVal *lValString(const char *c){
	return lValStringLen(c, strlen(c));
}

/* Create a new string value out of S, using C directly, which will be
 * freed once the value leaves scope  */
lVal *lValStringNoCopy(const char *c,int len){
	if(c == NULL){return NULL;}
	lVal *t = lRootsValPush(lValAlloc());
	t->type = ltString;
	t->vString = lStringNewNoCopy(c,len);
	return t;
}

/* Return a string value, containing a mark from ERR to ERREND,
 * which has to be within BUF and BUFSTART */
lVal *lValStringError(const char *bufStart, const char *bufEnd, const char *errStart, const char *err, const char *errEnd){
	char buf[512];
	const char *lineStart, *lineEnd;
	for(lineStart = errStart; (lineStart > bufStart) && (*lineStart != '\n'); lineStart--){}
	if(*lineStart == '\n'){lineStart++;}
	for(lineEnd = errEnd; (lineEnd < bufEnd) && (*lineEnd != '\n'); lineEnd++){}

	char *data = buf;
	if((errStart - lineStart) > 30){
		*data++ = '.';
		*data++ = '.';
		*data++ = '.';
		lineStart = errStart - 30;
	}
	while(lineStart < err){*data++ = *lineStart++;}
	*data = 0;
	const int sret = snprintf(data,sizeof(buf) - (data-buf), "\033[41m%c\033[49m",*err);
	if(sret > 0){
		data += sret;
	}else{
		return NULL;
	}
	bool endAbbreviated = false;
	if((lineEnd - errEnd) > 30){
		lineEnd = errEnd + 30;
		endAbbreviated = true;
	}
	lineStart = err+1;
	while(lineStart < lineEnd){*data++ = *lineStart++;}
	if(endAbbreviated){
		*data++ = '.';
		*data++ = '.';
		*data++ = '.';
	}
	*data = 0;
	return lValString(buf);
}
