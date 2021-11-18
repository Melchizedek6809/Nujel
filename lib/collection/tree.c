/*
 * This file is a part of Nujel, licensed under the MIT License.
 */
#include "tree.h"

#include "list.h"
#include "../allocation/tree.h"
#include "../allocation/garbage-collection.h"
#include "../allocation/roots.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <stdlib.h>

/* Create a new Tree segment with S associated to V */
static lTree *lTreeNew(const lSymbol *s, lVal *v){
	lTree *ret = lTreeAlloc();
	ret->key = s;
	ret->height = 1;
	ret->value = v;
	ret->left = NULL;
	ret->right = NULL;
	return ret;
}

/* Return the stored height of the tree T */
static uint lTreeHeight(const lTree *t){
	return t == NULL ? 0 : t->height;
}

/* Calculate the true height of tree T */
static uint lTreeCalcHeight(const lTree *t){
	return 1 + MAX(lTreeHeight(t->left),lTreeHeight(t->right));
}

/* Get the balance factor of T,
 * which is used to rebalance the tree automatically during inserts */
static int lTreeGetBalance(const lTree *t){
	return t == NULL ? 0 : lTreeHeight(t->left) - lTreeHeight(t->right);
}

/* Reset S to be associated to V if it has already been bound, storing TRUE in
 * FOUND on success */
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

/* Rotate the tree X to the left, in order to balance the tree again */
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

/* Rotate the tree X to the right, in order to balance the tree again */
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

/* Rebalance the tree T if is out of balance, S should be the last symbol that
 * was inserted, which must have led to the inbalance with it's insertion */
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

/* Insert an association S -> V in the tree T, creating a new segment if
 * necessary, otherwise the old segment will be mutated */
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

/* Get whatever value is associated in T to S,
 * setting FOUND to true if successful */
lVal *lTreeGet(const lTree *t, const lSymbol *s, bool *found){
	const lTree *c = t;
	while(c != NULL){
		if(s == c->key){
			if(found != NULL){*found = true;}
			return c->value;
		}
		c = s > c->key ? c->right : c->left;
	}
	if(found != NULL){*found = false;}
	return NULL;
}

/* Return true if there is an association for S in T, storing the value
 * in values, if found. */
bool lTreeHas(const lTree *t, const lSymbol *s, lVal **value){
	bool found = false;
	lVal *v = lTreeGet(t,s,&found);
	if((found) && value){*value = v;}
	return found;
}

/* Add every symbol/value pair within T onto list, making it a big a-list */
lVal *lTreeAddToList(const lTree *t, lVal *list){
	if(t == NULL){return list;}
	lRootsValPush(list);
	lVal *l = lRootsValPush(lCons(NULL,NULL));
	l->vList.cdr = lCons(NULL,lTreeAddToList(t->right,list));
	l->vList.cdr->vList.car = t->value;
	l->vList.car = lValSymS(t->key);
	return lTreeAddToList(t->left,l);
}

/* Add all the keys within T to the beginning LIST */
lVal *lTreeAddKeysToList(const lTree *t, lVal *list){
	if((t == NULL) || (t->key == NULL)){return list;}

	lRootsValPush(list);
	list = lTreeAddKeysToList(t->right,list);

	lVal *sym = lRootsValPush(lValSymS(t->key));
	list = lCons(sym,list);

	return lTreeAddKeysToList(t->left,list);
}
/* Add all the values within T to the beginning LIST */
lVal *lTreeAddValuesToList(const lTree *t, lVal *list){
	if(t == NULL){return list;}
	list = lTreeAddValuesToList(t->right,list);

	lRootsValPush(list);
	lVal *l = lCons(t->value,list);

	return lTreeAddValuesToList(t->left,l);
}

/* Create an a-list with all the key/value bindings from T */
lVal *lTreeToList(const lTree *t){
	return lTreeAddToList(t,NULL);
}

/* Create a list of all the keys within T */
lVal *lTreeKeysToList(const lTree *t){
	return lTreeAddKeysToList(t,NULL);
}

/* Create a list of all the values within T */
lVal *lTreeValuesToList(const lTree *t){
	return lTreeAddValuesToList(t,NULL);
}

/* Return the total size of the tree T */
uint lTreeSize(const lTree *t){
	return t == NULL ? 0 : (t->key ? 1 : 0) + lTreeSize(t->left) + lTreeSize(t->right);
}
