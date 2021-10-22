/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "string.h"

#include "array.h"
#include "closure.h"
#include "list.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"
#include "../type/vec.h"
#include "../nujel.h"
#include "../type-system.h"
#include "../allocator/garbage-collection.h"

#ifndef COSMOPOLITAN_H_
	#include <ctype.h>
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
#endif

lString  lStringList[STR_MAX];
uint     lStringActive = 0;
uint     lStringMax    = 0;
lString *lStringFFree  = NULL;

char *ansiRS = "\033[0m";
char *ansiFG[16] = {
	"\033[0;30m",
	"\033[0;31m",
	"\033[0;32m",
	"\033[0;33m",
	"\033[0;34m",
	"\033[0;35m",
	"\033[0;36m",
	"\033[0;37m",
	"\033[1;30m",
	"\033[1;31m",
	"\033[1;32m",
	"\033[1;33m",
	"\033[1;34m",
	"\033[1;35m",
	"\033[1;36m",
	"\033[1;37m"
};

void lInitStr(){
	lStringActive = 0;
	lStringMax    = 0;
}

lString *lStringAlloc(){
	lString *ret;
	if(lStringFFree == NULL){
		if(lStringMax >= STR_MAX){
			lGarbageCollect();
			if(lStringFFree == NULL){
				lPrintError("lString OOM ");
				return 0;
			}else{
				ret = lStringFFree;
				lStringFFree = ret->nextFree;
			}
		}else{
			ret = &lStringList[lStringMax++];
		}
	}else{
		ret = lStringFFree;
		lStringFFree = ret->nextFree;
	}
	lStringActive++;
	*ret = (lString){0};
	return ret;
}

void lStringFree(lString *s){
	if(s == NULL){return;}
	lStringActive--;
	s->nextFree = lStringFFree;
	lStringFFree = s;
}

lString *lStringNew(const char *str, uint len){
	if(str == NULL){return 0;}
	lString *s = lStringAlloc();
	if(s == NULL){return 0;}
	char *nbuf = malloc(len+1);
	if(nbuf == NULL){return 0;}
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	s->buf    = s->data = nbuf;
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
	if(t->vString == NULL){
		lValFree(t);
		return NULL;
	}
	return t;
}
lVal *lValCString(const char *c){
	if(c == NULL){return NULL;}
	lVal *t = lValAlloc();
	if(t == NULL){return NULL;}
	t->type = ltString;
	t->vString = lStringAlloc();
	if(t->vString == NULL){
		lValFree(t);
		return NULL;
	}
	t->vString->buf    = t->vString->data = c;
	t->vString->bufEnd = c + strlen(c);
	return t;
}
