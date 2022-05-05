/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "string.h"
#include "list.h"
#include "../display.h"
#include "../misc/pf.h"

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
	if(str == NULL){return NULL;}
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
	lVal *t = lRootsValPush(lValAlloc(ltString));
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
	if(c == NULL){return NULL;}
	lVal *t = lRootsValPush(lValAlloc(ltString));
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
	const char *msgEnd = MIN(lineEnd, (errEnd + 30));
	const size_t bufSize = (msgEnd - msgStart) + 3 + 3 + 1;
	char *outbuf = malloc(bufSize);
	char *outbufEnd = &outbuf[bufSize];
	char *data = outbuf;

	if(msgStart != lineStart){data = spf(data, outbufEnd, "...");}
	memcpy(data, msgStart, msgEnd - msgStart);
	data += (msgEnd - msgStart);
	if(msgEnd != lineEnd){data = spf(data, outbufEnd, "...");}
	*data = 0;
	return lValStringNoCopy(outbuf, data - outbuf);
}
