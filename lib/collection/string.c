/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "string.h"

#include "array.h"
#include "closure.h"
#include "list.h"
#include "../nujel.h"
#include "../allocation/garbage-collection.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"
#include "../type-system.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	if(s->flags & HEAP_ALLOCATED){
		free((void *)s->buf);
	}
	s->flags = 0;
	s->nextFree = lStringFFree;
	lStringFFree = s;
}

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