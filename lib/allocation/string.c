/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "string.h"
#include "../display.h"

#include <stdlib.h>
#include <string.h>

lString  lStringList[STR_MAX];
uint     lStringActive = 0;
uint     lStringMax    = 0;
lString *lStringFFree  = NULL;

/* Initialize the string allocator */
void lStringInit(){
	lStringActive = 0;
	lStringMax    = 0;
}

/* Return a newly allocated, empty string */
lString *lStringAlloc(){
	lString *ret;
	if(lStringFFree == NULL){
		if(lStringMax >= STR_MAX){
			lGarbageCollect();
			if(lStringFFree == NULL){
				lPrintError("lString OOM ");
				exit(123);
			}else{
				goto allocateFromFreeList;
			}
		}else{
			ret = &lStringList[lStringMax++];
		}
	}else{
		allocateFromFreeList:
		ret = lStringFFree;
		lStringFFree = ret->nextFree;
	}
	lStringActive++;
	memset(ret, 0, sizeof(lString));
	return ret;
}

/* Free the lString S, should never be called directly outside the GC */
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
