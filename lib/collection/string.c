/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "string.h"

#include "closure.h"
#include "list.h"
#include "../display.h"
#include "../nujel.h"
#include "../allocation/array.h"
#include "../allocation/garbage-collection.h"
#include "../allocation/string.h"
#include "../allocation/val.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"
#include "../type-system.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

int lStringLength(const lString *s){
	return s->bufEnd - s->buf;
}

lVal *lValString(const char *c){
	if(c == NULL){return NULL;}
	lVal *t = lValAlloc();
	if(t == NULL){return NULL;}
	t->type = ltString;
	t->vString = lStringNew(c,strlen(c));
	return t->vString == NULL ? NULL : t;
}
