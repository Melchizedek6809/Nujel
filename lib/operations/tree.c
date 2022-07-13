/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../nujel-private.h"

lVal *lnfTreeNew(lClosure *c, lVal *v){
	lVal *ret = lValAlloc(ltTree);
	ret->vTree = lTreeNew(NULL, NULL);

	for(lVal *n = v; n; n = lCddr(n)){
		lVal *car = lCar(n);
		if(car == NULL){break;}
		ret->vTree = lTreeInsert(ret->vTree, requireSymbolic(c, car), lCadr(n));
	}
	return ret;
}

static lVal *lnfTreeGetList(lClosure *c, lVal *v){
	return lTreeToList(requireTree(c, lCar(v)));
}

static lVal *lnfTreeGetKeys(lClosure *c, lVal *v){
	return lTreeKeysToList(requireTree(c, lCar(v)));
}

static lVal *lnfTreeGetValues(lClosure *c, lVal *v){
	return lTreeValuesToList(requireTree(c, lCar(v)));
}

static lVal *lnfTreeGet(lClosure *c, lVal *v){
	return lTreeGet(requireTree(c, lCar(v)), requireSymbolic(c, lCadr(v)), NULL);
}

static lVal *lnfTreeHas(lClosure *c, lVal *v){
	return lValBool(lTreeHas(requireTree(c, lCar(v)), requireSymbolic(c, lCadr(v)), NULL));
}

static lVal *lnfTreeSet(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	if(car == NULL){car = lValTree(NULL);}
	lTree *tre = requireMutableTree(c, car);
	const lSymbol *key = requireSymbolic(c, lCadr(v));
	car->vTree = lTreeInsert(tre, key, lCaddr(v));
	return car;
}

static lVal *lnfTreeSize(lClosure *c, lVal *v){
	return lValInt(lTreeSize(requireTree(c, lCar(v))));
}

static lVal *lnfTreeDup(lClosure *c, lVal *v){
	(void)c;
	if((v == NULL)
		|| (v->type != ltPair)
		|| (v->vList.car == NULL)
		|| (v->vList.car->type != ltTree)){
		lExceptionThrowValClo("type-error","tree/dup can only be called with a tree as an argument", v, c);
	}
	lTree *tree = requireTree(c, lCar(v));
	tree = lTreeDup(tree);
	return lValTree(tree);
}

static lVal *lnfTreeKeyAst(lClosure *c, lVal *v){
	lTree *tree = requireTree(c, lCar(v));
	return tree ? lValKeywordS(tree->key) : NULL;
}

static lVal *lnfTreeValueAst(lClosure *c, lVal *v){
	lTree *tree = requireTree(c, lCar(v));
	return tree ? tree->value : NULL;
}

static lVal *lnfTreeLeftAst(lClosure *c, lVal *v){
	lTree *tree = requireTree(c, lCar(v));
	return (tree && tree->left) ? lValTree(tree->left) : NULL;
}

static lVal *lnfTreeRightAst(lClosure *c, lVal *v){
	lTree *tree = requireTree(c, lCar(v));
	return (tree && tree->right) ? lValTree(tree->right) : NULL;
}

void lOperationsTree(lClosure *c){
	lAddNativeFunc(c,"tree/new",     "plist",          "Return a new tree", lnfTreeNew);
	lAddNativeFunc(c,"tree/ref",     "[tree sym]",     "Return the value of SYM in TREE, or #nil if not found", lnfTreeGet);
	lAddNativeFunc(c,"tree/list",    "[tree]",         "Return a TREE as a plist", lnfTreeGetList);
	lAddNativeFunc(c,"tree/keys",    "[tree]",         "Return each key of TREE in a list", lnfTreeGetKeys);
	lAddNativeFunc(c,"tree/values",  "[tree]",         "Return each value of TREE in a list", lnfTreeGetValues);
	lAddNativeFunc(c,"tree/size",    "[tree]",         "Return the amount of entries in TREE", lnfTreeSize);
	lAddNativeFunc(c,"tree/has?",    "[tree sym]",     "Return #t if TREE contains a value for SYM", lnfTreeHas);
	lAddNativeFunc(c,"tree/set!",    "[tree sym val]", "Set SYM to VAL in TREE", lnfTreeSet);
	lAddNativeFunc(c,"tree/dup",     "[tree]",         "Return a duplicate of TREE", lnfTreeDup);

	lAddNativeFunc(c,"tree/key*",    "[tree]",         "Low-level: return the key for TREE segment", lnfTreeKeyAst);
	lAddNativeFunc(c,"tree/value*",  "[tree]",         "Low-level: return the value for TREE segment", lnfTreeValueAst);
	lAddNativeFunc(c,"tree/left*",   "[tree]",         "Low-level: return the left ref for TREE segment", lnfTreeLeftAst);
	lAddNativeFunc(c,"tree/right*",  "[tree]",         "Low-level: return the right ref for TREE segment", lnfTreeRightAst);
}
