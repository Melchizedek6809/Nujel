/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "symbol.h"
#include "val.h"
#include "../display.h"
#include "../nujel.h"
#include "../allocation/symbol.h"
#include "../allocation/allocator.h"

#include <string.h>

/* Return a newly allocated nujel symbol of value S */
lVal *lValSymS(const lSymbol *s){
	if(s == NULL){return NULL;}
	lVal *ret = lValAlloc(ltSymbol);
	ret->vSymbol = s;
	return ret;
}

/* Return a nujel value for the symbol within S */
lVal *lValSym(const char *s){
	return lValSymS(RSYMP(lSymS(s)));
}

/* Return a newly allocated nujel keyword of value S */
lVal *lValKeywordS(const lSymbol *s){
	if(s == NULL){return NULL;}
	lVal *ret = lValAlloc(ltKeyword);
	ret->vSymbol = s;
	return ret;
}

/* Return a nujel value for the keyword within S */
lVal *lValKeyword(const char *s){
	return lValKeywordS(RSYMP(lSymS(s)));
}
