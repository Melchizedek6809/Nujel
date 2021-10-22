/*
 * This file is a part of Nujel, licensed under the MIT License.
 */
#include "tree.h"

#include "list.h"
#include "../allocation/garbage-collection.h"
#include "../allocation/roots.h"
#include "../type/symbol.h"
#include "../type/val.h"

#ifndef COSMOPOLITAN_H_
	#include <stdlib.h>
#endif

lTree    lTreeList[TRE_MAX];
uint     lTreeActive = 0;
uint     lTreeMax    = 0;
lTree   *lTreeFFree  = NULL;

void lTreeInit(){
	lTreeActive  = 0;
	lTreeMax     = 0;
}

lTree *lTreeAlloc(){
	lTree *ret;
	if(lTreeFFree == NULL){
		if(lTreeMax >= TRE_MAX-1){
			lGarbageCollect();
			if(lTreeFFree == NULL){
				lPrintError("lTree OOM\n");
				exit(1);
			}else{
				ret       = lTreeFFree;
				lTreeFFree = ret->nextFree;
			}
		}else{
			ret = &lTreeList[lTreeMax++];
		}
	}else{
		ret       = lTreeFFree;
		lTreeFFree = ret->nextFree;
	}
	lTreeActive++;
	*ret = (lTree){0};
	return ret;
}

lTree *lTreeNew(const lSymbol *s, lVal *v){
	lTree *ret = lTreeAlloc();
	ret->key = s;
	ret->value = v;
	return ret;
}

void lTreeFree(lTree *t){
	if(t == NULL){return;}
	lTreeActive--;
	t->nextFree = lTreeFFree;
	lTreeFFree = t;
}

void lTreeInsert(lTree *t, const lSymbol *s, lVal *v){
	lTree *c = t;
	if(s == NULL){return;}
	while(c != NULL){
		if(c->key == NULL){
			c->key   = s;
			c->value = v;
			return;
		}
		if(s == c->key){
			c->value = v;
			return;
		}
		if(s > c->key){
			if(c->right == NULL){
				c->right = lTreeNew(s,v);
				break;
			}
			c = c->right;
		}else{
			if(c->left == NULL){
				c->left = lTreeNew(s,v);
				break;
			}
			c = c->left;
		}
	}
}

lVal *lTreeGet(const lTree *t, const lSymbol *s){
	const lTree *c = t;
	while(c != NULL){
		if(c->key == NULL){break;}
		if(s == c->key){return c->value;}
		c = s > c->key ? c->right : c->left;
	}
	return NULL;
}

bool lTreeHas(const lTree *t, const lSymbol *s){
	const lTree *c = t;
	while(c != NULL){
		if(c->key == NULL){break;}
		if(s == c->key){return true;}
		c = s > c->key ? c->right : c->left;
	}
	return false;
}

lVal *lTreeAddToList(const lTree *t, lVal *list){
	if(t == NULL){return list;}
	if(list != NULL){lRootsValPush(list);}
	lVal *l = lRootsValPush(lCons(NULL,NULL));
	l->vList.cdr = lCons(NULL,lTreeAddToList(t->right,list));
	l->vList.cdr->vList.car = t->value;
	l->vList.car = lValSymS(t->key);
	if(list != NULL){lRootsValPop();}
	lRootsValPop();
	return lTreeAddToList(t->left,l);
}

lVal *lTreeAddKeysToList(const lTree *t, lVal *list){
	if(t == NULL){return list;}
	list = lTreeAddKeysToList(t->right,list);

	if(list != NULL){lRootsValPush(list);}
	lVal *l = lCons(NULL,list);
	if(list != NULL){lRootsValPop();}

	lRootsValPush(l);
	l->vList.car = lValSymS(t->key);
	lRootsValPop();

	return lTreeAddKeysToList(t->left,l);
}

lVal *lTreeAddValuesToList(const lTree *t, lVal *list){
	if(t == NULL){return list;}
	list = lTreeAddValuesToList(t->right,list);

	if(list != NULL){lRootsValPush(list);}
	lVal *l = lCons(t->value,list);
	if(list != NULL){lRootsValPop();}

	return lTreeAddValuesToList(t->left,l);
}

lVal *lTreeToList(const lTree *t){
	return lTreeAddToList(t,NULL);
}

lVal *lTreeKeysToList(const lTree *t){
	return lTreeAddKeysToList(t,NULL);
}

lVal *lTreeValuesToList(const lTree *t){
	return lTreeAddValuesToList(t,NULL);
}
