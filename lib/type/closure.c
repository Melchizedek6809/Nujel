/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../display.h"
#include "../nujel.h"
#include "../allocation/closure.h"
#include "../allocation/garbage-collection.h"
#include "../collection/list.h"
#include "../collection/tree.h"
#include "../type/closure.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <ctype.h>
#include <string.h>

/* Return a new closure, setting the parent field */
lClosure *lClosureNew(lClosure *parent){
	lClosure *c = lClosureAlloc();
	if(c == NULL){return NULL;}
	c->parent = parent;
	return c;
}

/* Define the NFunc LNF in C with all the space separated symbols in SYM */
lVal *lDefineAliased(lClosure *c, lVal *lNF, const char *sym){
	const char *cur = sym;

	// Run at most 64 times, just a precaution
	for(int i=0;i<64;i++){
		uint len;
		for(len=0;len < sizeof(lSymbol);len++){ // Find the end of the current token, either space or 0
			if(cur[len] == 0)    {break;}
				if(isspace((u8)cur[len])){break;}
		}
		lDefineClosureSym(c,lSymSL(cur,len),lNF);
		for(;len<32;len++){ // Advance to the next non whitespace character
			if(cur[len] == 0)     {return lNF;} // Or return if we reached the final 0 byte
			if(!isspace((u8)cur[len])){break;}
		}
		cur += len;
	}
	lPrintError("Quite the amount of aliases we have there (%s)\n",sym);
	return NULL;
}

/* Return TRUE if C contains a binding for S, storing the value in V */
bool lHasClosureSym(lClosure *c, const lSymbol *s, lVal **v){
	if((c == NULL) || (s == NULL)){return NULL;}
	bool found = false;
	lVal *t = lTreeGet(c->data,s,&found);
	if(found && (v != NULL)){
		*v = t;
	}
	return found ? true : lHasClosureSym(c->parent,s,v);
}

/* Return the value bound to S in C */
lVal *lGetClosureSym(lClosure *c, const lSymbol *s){
	lVal *ret;
	return lHasClosureSym(c,s,&ret) ? ret : NULL;
}

/* Bind the value V to the Symbol S in the closure C, defining it if necessary */
void lDefineClosureSym(lClosure *c, const lSymbol *s, lVal *v){
	c->data = lTreeInsert(c->data,s,v);
}

/* Set the value bound to S in C to V, if it has already been bounbd */
void lSetClosureSym(lClosure *c, const lSymbol *s, lVal *v){
	if(c == NULL){return;}
	bool found = false;
	lTreeSet(c->data,s,v,&found);
	if(found){return;}
	lSetClosureSym(c->parent,s,v);
}

/* Turn STR into a symbol, and bind VAL to it within C */
void lDefineVal(lClosure *c, const char *str, lVal *val){
	lDefineClosureSym(c,lSymS(str),val);
}
