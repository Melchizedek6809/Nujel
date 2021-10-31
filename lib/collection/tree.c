/*
 * This file is a part of Nujel, licensed under the MIT License.
 */
#include "tree.h"

#include "list.h"
#include "../allocation/garbage-collection.h"
#include "../allocation/roots.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <stdlib.h>

lTree    lTreeList[TRE_MAX];
uint     lTreeActive = 0;
uint     lTreeMax    = 0;
lTree   *lTreeFFree  = NULL;

void lTreeInit(){
	lTreeActive = 0;
	lTreeMax    = 0;
}

static lTree *lTreeAlloc(){
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
	return ret;
}

static lTree *lTreeNew(const lSymbol *s, lVal *v){
	lTree *ret = lTreeAlloc();
	ret->key = s;
	ret->height = 1;
	ret->value = v;
	ret->left = NULL;
	ret->right = NULL;
	return ret;
}

static uint lTreeHeight(const lTree *t){
	return t == NULL ? 0 : t->height;
}

static uint lTreeCalcHeight(const lTree *t){
	return 1 + MAX(lTreeHeight(t->left),lTreeHeight(t->right));
}

static int lTreeGetBalance(const lTree *t){
	return t == NULL ? 0 : lTreeHeight(t->left) - lTreeHeight(t->right);
}

void lTreeFree(lTree *t){
	if(t == NULL){return;}
	lTreeActive--;
	t->nextFree = lTreeFFree;
	lTreeFFree = t;
}

void lTreeSet(lTree *t, const lSymbol *s, lVal *v, bool *found){
	if(t == NULL){
		return;
	}else if(s == t->key){
		t->value = v;
		if(found){*found = true;}
	}else if(s > t->key){
		lTreeSet(t->right,s,v,found);
	}else{
		lTreeSet(t->left,s,v,found);
	}
}

static lTree *lTreeRotateLeft(lTree *x){
	lTree *y = x->right;
	if(y == NULL){return x;}
	lTree *T2 = y->left;

	y->left = x;
	x->right = T2;

	x->height = lTreeCalcHeight(x);
	y->height = lTreeCalcHeight(y);

	return y;
}

static lTree *lTreeRotateRight(lTree *y){
	lTree *x = y->left;
	if(x == NULL){return y;}
	lTree *T2 = x->right;

	x->right = y;
	y->left = T2;

	x->height = lTreeCalcHeight(x);
	y->height = lTreeCalcHeight(y);

	return x;
}

static lTree *lTreeBalance(lTree *t, const lSymbol *s){
	int balance = lTreeGetBalance(t);

	if(balance < -1){
		if(s < t->right->key){
			t->right = lTreeRotateRight(t->right);
		}
		return lTreeRotateLeft(t);
	}else if(balance > 1){
		if(s > t->left->key){
			t->left = lTreeRotateLeft(t->left);
		}
		return lTreeRotateRight(t);
	}
	return t;
}

lTree *lTreeInsert(lTree *t, const lSymbol *s, lVal *v){
	if(t == NULL){
		return lTreeNew(s,v);
	}else if(t->key == s){
		t->value = v;
		return t;
	}else{
		if(s < t->key){
			t->left = lTreeInsert(t->left,s,v);
		}else {
			t->right = lTreeInsert(t->right,s,v);
		}
		t->height = lTreeCalcHeight(t);
		t = lTreeBalance(t,s);
		return t;
	}
}

lVal *lTreeGet(const lTree *t, const lSymbol *s, bool *found){
	const lTree *c = t;
	while(c != NULL){
		if(c->key == NULL){break;}
		if(s == c->key){
			if(found != NULL){*found = true;}
			return c->value;
		}
		c = s > c->key ? c->right : c->left;
	}
	if(found != NULL){*found = false;}
	return NULL;
}

bool lTreeHas(const lTree *t, const lSymbol *s, lVal **value){
	const lTree *c = t;
	while(c != NULL){
		if(c->key == NULL){break;}
		if(s == c->key){
			if(value != NULL){*value = c->value;}
			return true;
		}
		c = s > c->key ? c->right : c->left;
	}
	if(value != NULL){*value = NULL;}
	return false;
}

lVal *lTreeAddToList(const lTree *t, lVal *list){
	if(t == NULL){return list;}
	lRootsValPush(list);
	lVal *l = lRootsValPush(lCons(NULL,NULL));
	l->vList.cdr = lCons(NULL,lTreeAddToList(t->right,list));
	l->vList.cdr->vList.car = t->value;
	l->vList.car = lValSymS(t->key);
	return lTreeAddToList(t->left,l);
}

lVal *lTreeAddKeysToList(const lTree *t, lVal *list){
	if((t == NULL) || (t->key == NULL)){return list;}

	lRootsValPush(list);
	list = lTreeAddKeysToList(t->right,list);

	lVal *sym = lRootsValPush(lValSymS(t->key));
	list = lCons(sym,list);

	return lTreeAddKeysToList(t->left,list);
}

lVal *lTreeAddValuesToList(const lTree *t, lVal *list){
	if(t == NULL){return list;}
	list = lTreeAddValuesToList(t->right,list);

	lRootsValPush(list);
	lVal *l = lCons(t->value,list);

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

uint lTreeSize(const lTree *t){
	return t == NULL ? 0 : (t->key ? 1 : 0) + lTreeSize(t->left) + lTreeSize(t->right);
}
