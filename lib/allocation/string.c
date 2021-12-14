/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "string.h"

#include "../display.h"
#include "../nujel.h"
#include "../allocation/array.h"
#include "../allocation/garbage-collection.h"
#include "../allocation/val.h"
#include "../collection/list.h"
#include "../type/closure.h"
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
