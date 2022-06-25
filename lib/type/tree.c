/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "tree.h"
#include "../type/val.h"

lTree *lTreeNew(const lSymbol *s, lVal *v){
	lTree *ret = lTreeAlloc();
	ret->key    = s;
	ret->height = 1;
	ret->value  = v;
	return ret;
}

static uint lTreeHeight(const lTree *t){
	return unlikely(t == NULL) ? 0 : t->height;
}

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
	}else if(unlikely(t->key == NULL)){
		t->key = s;
		t->value = v;
		return t;
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
	while(c){
		if(s == c->key){
			if(found != NULL){*found = true;}
			return c->value;
		}
		c = s > c->key ? c->right : c->left;
	}
	return NULL;
}

/* Return true if there is an association for S in T, storing the value
 * in values, if found. */
bool lTreeHas(const lTree *t, const lSymbol *s, lVal **value){
	bool found = false;
	lVal *v = lTreeGet(t, s, &found);
	if(found && value){*value = v;}
	return found;
}

/* Add every symbol/value pair within T onto list, making it a big a-list */
static lVal *lTreeAddToList(const lTree *t, lVal *list){
	if(unlikely((t == NULL) || (t->key == NULL))){return list;}
	lVal *l = lCons(NULL,NULL);
	l->vList.cdr = lCons(NULL,lTreeAddToList(t->right,list));
	l->vList.cdr->vList.car = t->value;
	l->vList.car = lValKeywordS(t->key);
	return lTreeAddToList(t->left,l);
}

/* Add all the keys within T to the beginning LIST */
static lVal *lTreeAddKeysToList(const lTree *t, lVal *list){
	if(unlikely((t == NULL) || (t->key == NULL))){return list;}
	list = lTreeAddKeysToList(t->right, list);
	list = lCons(lValKeywordS(t->key), list);
	return lTreeAddKeysToList(t->left, list);
}
/* Add all the values within T to the beginning LIST */
static lVal *lTreeAddValuesToList(const lTree *t, lVal *list){
	if(unlikely((t == NULL) || (t->key == NULL))){return list;}
	list = lTreeAddValuesToList(t->right, list);
	list = lCons(t->value, list);
	return lTreeAddValuesToList(t->left, list);
}

/* Create an a-list with all the key/value bindings from T */
lVal *lTreeToList(const lTree *t){
	return t ? lTreeAddToList(t,NULL) : NULL;
}

/* Create a list of all the keys within T */
lVal *lTreeKeysToList(const lTree *t){
	return t ? lTreeAddKeysToList(t,NULL) : NULL;
}

/* Create a list of all the values within T */
lVal *lTreeValuesToList(const lTree *t){
	return t ? lTreeAddValuesToList(t,NULL) : NULL;
}

/* Return the total size of the tree T */
uint lTreeSize(const lTree *t){
	return t == NULL ? 0 : (t->key ? 1 : 0) + lTreeSize(t->left) + lTreeSize(t->right);
}

/* Return a duplicate of t */
lTree *lTreeDup(const lTree *t){
	if(unlikely(t == NULL)){return NULL;}
	lTree *ret  = lTreeAlloc();
	ret->key    = t->key;
	ret->value  = t->value;
        ret->height = t->height;
	ret->left   = lTreeDup(t->left);
	ret->right  = lTreeDup(t->right);
	return ret;
}
