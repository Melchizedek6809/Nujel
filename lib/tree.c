/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

lTree *lTreeNew(const lSymbol *s, lVal v){
	lTree *ret  = lTreeAllocRaw();
	ret->key    = s;
	ret->height = 1;
	ret->value  = v;
	return ret;
}

static uint lTreeHeight(const lTree *t){
	return t == NULL ? 0 : t->height;
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
bool lTreeSet(lTree *t, const lSymbol *s, lVal v){
	lTree *c = t;
	while(c != NULL){
		if(c->key == s){
			c->value = v;
			return true;
		}
		c = s > c->key ? c->right : c->left;
	}
	return false;
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
lTree *lTreeInsert(lTree *t, const lSymbol *s, lVal v){
	if(unlikely(t == NULL)){
		return lTreeNew(s,v);
	}else if(unlikely(t->key == NULL)){
		t->key = s;
		t->value = v;
		return t;
	}else if(unlikely(t->key == s)){
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
 * Returns a simple Exception if nothing is found */
lVal lTreeRef(const lTree *t, const lSymbol *s){
	const lTree *c = t;
	while(c){
		if(s == c->key){
			return c->value;
		}
		c = s > c->key ? c->right : c->left;
	}
	return lValExceptionSimple();;
}

/* Add all the keys within T to the beginning LIST */
static lVal lTreeAddKeysToList(const lTree *t, lVal list){
	if(unlikely((t == NULL) || (t->key == NULL))){return list;}
	list = lTreeAddKeysToList(t->right, list);
	list = lCons(lValKeywordS(t->key), list);
	return lTreeAddKeysToList(t->left, list);
}
/* Add all the values within T to the beginning LIST */
static lVal lTreeAddValuesToList(const lTree *t, lVal list){
	if(unlikely((t == NULL) || (t->key == NULL))){return list;}
	list = lTreeAddValuesToList(t->right, list);
	list = lCons(t->value, list);
	return lTreeAddValuesToList(t->left, list);
}

/* Create a list of all the keys within T */
static lVal lTreeKeysToList(const lTree *t){
	return t ? lTreeAddKeysToList(t,NIL) : NIL;
}

/* Create a list of all the values within T */
static lVal lTreeValuesToList(const lTree *t){
	return t ? lTreeAddValuesToList(t, NIL) : NIL;
}

/* Return the total size of the tree T */
static uint lTreeSize(const lTree *t){
	return t == NULL ? 0 : (t->key ? 1 : 0) + lTreeSize(t->left) + lTreeSize(t->right);
}

/* Return a duplicate of t */
lTree *lTreeDup(const lTree *t){
	if(unlikely(t == NULL)){return NULL;}
	lTree *ret  = lTreeAllocRaw();
	ret->key    = t->key;
	ret->value  = t->value;
        ret->height = t->height;
	ret->left   = lTreeDup(t->left);
	ret->right  = lTreeDup(t->right);
	return ret;
}

lVal lnfTreeNew(lClosure* c, lVal v) {
	(void)c;
	lTreeRoot *t = lTreeRootAllocRaw();
	lVal ret = lValAlloc(ltTree, t);

	for (lVal n = v; n.type == ltPair; n = lCddr(n)) {
		lVal car = lCar(n);
		if (car.type == ltNil) { break; }
		car = requireSymbolic(car);
		if(unlikely(car.type == ltException)){
			return car;
		}
		t->root = lTreeInsert(t->root, car.vSymbol, lCadr(n));
	}
	return ret;
}

static lVal lnfTreeGetKeys(lClosure* c, lVal v) {
	(void)c;
	lVal car = requireTree(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	return lTreeKeysToList(car.vTree->root);
}

static lVal lnfTreeGetValues(lClosure* c, lVal v) {
	(void)c;
	lVal car = requireTree(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	return lTreeValuesToList(car.vTree->root);
}

static lVal lnfTreeHas(lClosure* c, lVal v) {
	(void)c;
	lVal car = requireTree(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lVal cadr = requireSymbolic(lCadr(v));
	if(unlikely(cadr.type == ltException)){
		return cadr;
	}
	return lValBool(lTreeRef(car.vTree->root, cadr.vSymbol).type != ltException);
}

static lVal lnfTreeSize(lClosure* c, lVal v) {
	(void)c;
	lVal car = requireTree(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	return lValInt(lTreeSize(car.vTree->root));
}

static lVal lnfTreeDup(lClosure* c, lVal v) {
	(void)c;
	if (unlikely((v.type != ltPair)
		|| (v.vList->car.type != ltTree))) {
		return lValException("type-error", "tree/dup can only be called with a tree as an argument", v);
	}
	lVal car = requireTree(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lTree* tree = car.vTree->root;
	tree = lTreeDup(tree);
	return lValTree(tree);
}

static lVal lnfTreeKeyAst(lClosure* c, lVal v) {
	(void)c;
	lVal car = requireTree(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lTree* tree = car.vTree->root;
	return tree ? lValKeywordS(tree->key) : NIL;
}

static lVal lnfTreeValueAst(lClosure* c, lVal v) {
	(void)c;
	lVal car = requireTree(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lTree* tree = car.vTree->root;
	return tree ? tree->value : NIL;
}

static lVal lnfTreeLeftAst(lClosure* c, lVal v) {
	(void)c;
	lVal car = requireTree(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lTree* tree = car.vTree->root;
	return (tree && tree->left) ? lValTree(tree->left) : NIL;
}

static lVal lnfTreeRightAst(lClosure* c, lVal v) {
	(void)c;
	lVal car = requireTree(lCar(v));
	if(unlikely(car.type == ltException)){
		return car;
	}
	lTree* tree = car.vTree->root;
	return (tree && tree->right) ? lValTree(tree->right) : NIL;
}

void lOperationsTree(lClosure* c) {
	lAddNativeFunc(c, "tree/new",    "plist",          "Return a new tree", lnfTreeNew);
	lAddNativeFunc(c, "tree/keys",   "(tree)",         "Return each key of TREE in a list", lnfTreeGetKeys);
	lAddNativeFunc(c, "tree/values", "(tree)",         "Return each value of TREE in a list", lnfTreeGetValues);
	lAddNativeFunc(c, "tree/size",   "(tree)",         "Return the amount of entries in TREE", lnfTreeSize);
	lAddNativeFunc(c, "tree/has?",   "(tree sym)",     "Return #t if TREE contains a value for SYM", lnfTreeHas);
	lAddNativeFunc(c, "tree/dup",    "(tree)",         "Return a duplicate of TREE", lnfTreeDup);

	lAddNativeFunc(c, "tree/key*",   "(tree)", "Low-level: return the key for TREE segment", lnfTreeKeyAst);
	lAddNativeFunc(c, "tree/value*", "(tree)", "Low-level: return the value for TREE segment", lnfTreeValueAst);
	lAddNativeFunc(c, "tree/left*",  "(tree)", "Low-level: return the left ref for TREE segment", lnfTreeLeftAst);
	lAddNativeFunc(c, "tree/right*", "(tree)", "Low-level: return the right ref for TREE segment", lnfTreeRightAst);
}
