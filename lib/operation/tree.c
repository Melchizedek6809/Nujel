/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "tree.h"
#include "../exception.h"
#include "../nujel.h"
#include "../allocation/roots.h"
#include "../allocation/val.h"
#include "../collection/list.h"
#include "../collection/tree.h"
#include "../type/native-function.h"
#include "../type/val.h"
#include "../type-system.h"

lVal *lnfvTreeGet;

/* [tree/new ...plist] - Return a new tree, initialized with PLIST */
static lVal *lnfTreeNew(lClosure *c, lVal *v){
	(void)c; (void) v;

	lVal *ret = lRootsValPush(lValAlloc());
	ret->type = ltTree;
	ret->vTree = NULL;

	while(v != NULL){
		lVal *key = lCar(v);
		lVal *val = lCadr(v);
		if((key == NULL) || (key->type != ltSymbol)){break;}
		ret->vTree = lTreeInsert(ret->vTree,key->vSymbol,val);
		v = lCddr(v);
	}

	return ret;
}

/* [tree/list tree] - eturn a TREE as a plist */
static lVal *lnfTreeGetList(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if(car->type != ltTree){return NULL;}
	return lTreeToList(car->vTree);
}

/* [tree/keys tree] - Return each key of TREE in a list */
static lVal *lnfTreeGetKeys(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if(car->type != ltTree){return NULL;}
	return lTreeKeysToList(car->vTree);
}

/* [tree/values tree] - Return each value of TREE in a list */
static lVal *lnfTreeGetValues(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if(car->type != ltTree){return NULL;}
	return lTreeValuesToList(car->vTree);
}

/* [tree/get tree sym] - Return the value of SYM in TREE, or #nil */
lVal *lnfTreeGet(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if((car == NULL) || (car->type != ltTree)){return NULL;}
	lTree *tre = car->vTree;
	lVal *vs = lCadr(v);
	if((vs == NULL) || (vs->type != ltSymbol)){return car;}
	return lTreeGet(tre,vs->vSymbol,NULL);
}

/* [tree/has? tree sym] - Return #t if TREE contains a value for SYM */
static lVal *lnfTreeHas(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if((car == NULL) || (car->type != ltTree)){return NULL;}
	lTree *tre = car->vTree;
	lVal *vs = lCadr(v);
	if((vs == NULL) || (vs->type != ltSymbol)){return NULL;}
	return lValBool(lTreeHas(tre,vs->vSymbol,NULL));
}

/* [tree/set! tree sym val] - Set SYM to VAL in TREE */
static lVal *lnfTreeSet(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if((car == NULL) || (car->type != ltTree)){return NULL;}
	lTree *tre = car->vTree;
	lVal *vs = lCadr(v);
	if((vs == NULL) || (vs->type != ltSymbol)){return NULL;}
	car->vTree = lTreeInsert(tre,vs->vSymbol,lCaddr(v));
	return car;
}

/* [tree/size tree] - Return the amount of entries in TREE */
static lVal *lnfTreeSize(lClosure *c, lVal *v){
	(void)c;
	lTree *tree = castToTree(lCar(v),NULL);
	return lValInt(tree == 0 ? 0 : lTreeSize(tree));
}

static lVal *lnfTreeDup(lClosure *c, lVal *v){
	(void)c;
	if((v == NULL)
		|| (v->type != ltPair)
		|| (v->vList.car == NULL)
		|| (v->vList.car->type != ltTree)){
		lExceptionThrowValClo(":type-error","tree/dup can only be called with a tree as an argument", v, c);
	}
	lTree *tree = castToTree(lCar(v),NULL);
	if(!tree){return lCar(v);}
	return lValTree(lTreeDup(tree));
}

void lOperationsTree(lClosure *c){
	lnfvTreeGet = lAddNativeFunc(c,"tree/get","[tree sym]","Return the value of SYM in TREE, or #nil", lnfTreeGet);
	lAddNativeFunc(c,"tree/new",     "[...plist]",     "Return a new tree", lnfTreeNew);
	lAddNativeFunc(c,"tree/list",    "[tree]",         "Return a TREE as a plist", lnfTreeGetList);
	lAddNativeFunc(c,"tree/keys",    "[tree]",         "Return each key of TREE in a list", lnfTreeGetKeys);
	lAddNativeFunc(c,"tree/values",  "[tree]",         "Return each value of TREE in a list", lnfTreeGetValues);
	lAddNativeFunc(c,"tree/get-list","[tree]",         "Return a TREE as a plist", lnfTreeGetList);
	lAddNativeFunc(c,"tree/size",    "[tree]",         "Return the amount of entries in TREE", lnfTreeSize);
	lAddNativeFunc(c,"tree/has?",    "[tree sym]",     "Return #t if TREE contains a value for SYM", lnfTreeHas);
	lAddNativeFunc(c,"tree/set!",    "[tree sym val]", "Set SYM to VAL in TREE", lnfTreeSet);
	lAddNativeFunc(c,"tree/dup",     "[tree]",         "Return a duplicate of TREE", lnfTreeDup);
}
