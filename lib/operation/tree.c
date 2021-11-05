/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "tree.h"
#include "../nujel.h"
#include "../allocation/roots.h"
#include "../allocation/val.h"
#include "../collection/list.h"
#include "../collection/tree.h"
#include "../type/native-function.h"
#include "../type/val.h"
#include "../type-system.h"

lVal *lnfvTreeGet;

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

static lVal *lnfTreeGetList(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if(car->type != ltTree){return NULL;}
	lTree *tre = car->vTree;
	return lTreeToList(tre);
}

static lVal *lnfTreeGetKeys(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if(car->type != ltTree){return NULL;}
	lTree *tre = car->vTree;
	return lTreeKeysToList(tre);
}

static lVal *lnfTreeGetValues(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if(car->type != ltTree){return NULL;}
	lTree *tre = car->vTree;
	return lTreeValuesToList(tre);
}

lVal *lnfTreeGet(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if((car == NULL) || (car->type != ltTree)){return NULL;}
	lTree *tre = car->vTree;
	lVal *vs = lCadr(v);
	if((vs == NULL) || (vs->type != ltSymbol)){return car;}
	return lTreeGet(tre,vs->vSymbol,NULL);
}

static lVal *lnfTreeHas(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if((car == NULL) || (car->type != ltTree)){return NULL;}
	lTree *tre = car->vTree;
	lVal *vs = lCadr(v);
	if((vs == NULL) || (vs->type != ltSymbol)){return NULL;}
	return lValBool(lTreeHas(tre,vs->vSymbol,NULL));
}

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

static lVal *lnfTreeSize(lClosure *c, lVal *v){
	(void)c;
	lTree *tree = castToTree(lCar(v),NULL);
	return lValInt(tree == 0 ? 0 : lTreeSize(tree));
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
}
