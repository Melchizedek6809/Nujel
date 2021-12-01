/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "closure.h"
#include "special.h"
#include "../allocation/symbol.h"
#include "../display.h"
#include "../exception.h"
#include "../type-system.h"
#include "../allocation/roots.h"
#include "../allocation/val.h"
#include "../collection/list.h"
#include "../collection/tree.h"
#include "../type/closure.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"

const lSymbol *symType;
const lSymbol *symDocumentation;
const lSymbol *symArguments;
const lSymbol *symCode;
const lSymbol *symData;

static lVal *lnfDef(lClosure *c, lVal *v){
	lVal *sym = lCar(v);
	if(sym == NULL){return NULL;}
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return NULL;}
	const lSymbol *s = sym->vSymbol;

	lVal *ret = lEval(c,lCadr(v));
	lDefineClosureSym(c,s,ret);
	return ret;
}

static lVal *lnfSet(lClosure *c, lVal *v){
	lVal *sym = lCar(v);
	if(sym == NULL){return NULL;}
	if(sym->type != ltSymbol){sym = lEval(c,sym);}
	if(sym->type != ltSymbol){return NULL;}
	const lSymbol *s = sym->vSymbol;

	lVal *ret = lEval(c,lCadr(v));
	lSetClosureSym(c,s,ret);
	return ret;
}

static lVal *lSymTable(lClosure *c, lVal *v){
	if(c == NULL){return v;}
	lRootsValPush(v);
	v = lTreeAddKeysToList(c->data,v);
	return lSymTable(c->parent,v);
}

static lVal *lnfSymbolTable(lClosure *c, lVal *v){
	(void)v;
	return lSymTable(c,NULL);
}

static int lSymCount(lClosure *c, int ret){
	if(c == NULL){return ret;}
	return lSymCount(c->parent,lTreeSize(c->data) + ret);
}

static lVal *lnfSymCount(lClosure *c, lVal *v){
	(void)v;
	return lValInt(lSymCount(c,0));
}

static lVal *lnfClosure(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if((car == NULL)
		|| !((car->type == ltSpecialForm)
		||   (car->type == ltNativeFunc)
		||   (car->type == ltLambda)
		||   (car->type == ltObject)
		||   (car->type == ltMacro))){
		return NULL;
	}
	lVal *ret = lRootsValPush(lValTree(NULL));
	ret->vTree = lTreeInsert(ret->vTree,lSymS(":type"),lRootsValPush(lValSymS(getTypeSymbol(car))));
	if((car->type == ltSpecialForm) || (car->type == ltNativeFunc)){
		lNFunc *nf = car->vNFunc;
		ret->vTree = lTreeInsert(ret->vTree,symDocumentation,nf->doc);
		ret->vTree = lTreeInsert(ret->vTree,symArguments,nf->args);
		ret->vTree = lTreeInsert(ret->vTree,lSymS(":name"),lValSymS(nf->name));
	}else{
		lClosure *clo = car->vClosure;
		ret->vTree = lTreeInsert(ret->vTree,lSymS(":name"),lValSymS(clo->name));
		ret->vTree = lTreeInsert(ret->vTree,lSymS(":documentation"),clo->doc);
		ret->vTree = lTreeInsert(ret->vTree,lSymS(":arguments"),clo->args);
		ret->vTree = lTreeInsert(ret->vTree,lSymS(":code"),clo->text);
		ret->vTree = lTreeInsert(ret->vTree,lSymS(":data"), lRootsValPush(lValTree(clo->data)));
		if(clo->type == closureCall){
			ret->vTree = lTreeInsert(ret->vTree,lSymS(":call"), lRootsValPush(lValBool(true)));
		}
	}
	return ret;
}

static lVal *lnfClosureParent(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if((car == NULL)
		|| !((car->type == ltLambda)
		||   (car->type == ltObject)
		||   (car->type == ltMacro))){
		return NULL;
	}else if(car->vClosure->parent == NULL){
		return NULL;
	}else{
		lVal *ret = lValAlloc();
		ret->vClosure = car->vClosure->parent;
		ret->type = ret->vClosure->type == closureObject ? ltObject : ltLambda;
		return ret;
	}
}

static lVal *lnfClosureCaller(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	if((car == NULL)
		|| !((car->type == ltLambda)
		||   (car->type == ltObject)
		||   (car->type == ltMacro))){
		return NULL;
	}else if(car->vClosure->caller == NULL){
		return NULL;
	}else{
		lVal *ret = lValAlloc();
		ret->vClosure = car->vClosure->caller;
		ret->type = ret->vClosure->type == closureObject ? ltObject : ltLambda;
		return ret;
	}
}

static void lClosureSetRec(lClosure *clo, lTree *data){
	if(data == NULL){return;}
	const lSymbol *sym = data->key;
	if(data->key == symDocumentation){
		clo->doc = data->value;
	}else if(data->key == symArguments){
		clo->args = data->value;
	}else if(data->key == symCode){
		clo->text = data->value;
	}else if(data->key == symData){
		lTree *newData = castToTree(data->value,NULL);
		if(newData){
			clo->data = newData;
		}
	}else {
		lExceptionThrowValClo(":invalid-field","Trying to set an unknown or forbidden field for a closure", lValSymS(sym), clo);
	}
	lClosureSetRec(clo,data->left);
	lClosureSetRec(clo,data->right);
}

static lVal *lnfClosureSet(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	lTree *data = castToTree(lCadr(v),NULL);
	if((car == NULL) || (data == NULL)
		|| !((car->type == ltLambda)
		||   (car->type == ltObject)
		||   (car->type == ltMacro))){
		return NULL;
	}
	lClosure *clo = car->vClosure;
	lClosureSetRec(clo,data);
	return NULL;
}

static lVal *lnfLetRaw(lClosure *c, lVal *v){
	const int SP = lRootsGet();
	lClosure *nc = lRootsClosurePush(lClosureNew(c));
	nc->name     = c->name;
	nc->caller   = c->caller;
	nc->type     = c->type;
	lVal *ret    = lnfDo(nc,v);
	lRootsRet(SP);
	return ret;
}

static lClosure *getNextObject(lClosure *c){
	while(c != NULL){
		if(c->type == closureConstant){return NULL;}
		if(c->type == closureObject){return c;}
		c = c->parent;
	}
	return NULL;
}

static lVal *lnfClSelf(lClosure *c, lVal *v){
	c = getNextObject(c);
	for(int i=castToInt(lCar(v),0);i>0;i--){
		if(c == NULL){return NULL;}
		c = getNextObject(c->parent);
	}
	if(c == NULL){return NULL;}
	lVal *ret = lValAlloc();
	ret->type = ltObject;
	ret->vClosure = c;
	return ret;
}

static lVal *lnfResolve(lClosure *c, lVal *v){
	const lSymbol *sym = castToSymbol(lCar(v),NULL);
	return sym ? lGetClosureSym(c,sym) : NULL;
}

static lVal *lnfResolvesPred(lClosure *c, lVal *v){
	const lSymbol *sym = castToSymbol(lCar(v),NULL);
	return lValBool(sym ? lHasClosureSym(c,sym,NULL) : false);
}

/* Handler for [λ$ [..args] docstring body] */
static lVal *lnfLambdaAst(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	if(v == NULL){
		lExceptionThrowValClo(":invalid-lambda","Lambdas do need to have a name, some arguments a docstring and a body",NULL, c);
		return NULL;
	}
	const lSymbol *name = (car && car->type == ltSymbol) ? car->vSymbol : NULL;
	lVal *ret = lRootsValPush(lValAlloc());
	ret->type           = ltLambda;
	ret->vClosure       = lClosureNew(c);

	ret->vClosure->name = name;
	ret->vClosure->args = lCadr(v);
	ret->vClosure->doc  = lCaddr(v);
	ret->vClosure->text = lCadddr(v);

	return ret;
}

/* Handler for [μ* [...args] ...body] */
static lVal *lnfMacroAst(lClosure *c, lVal *v){
	lVal *ret = lnfLambdaAst(c,v);
	if(ret){ ret->type = ltMacro; }
	return ret;
}

/* Handler for [ω ...body] */
static lVal *lnfObject(lClosure *c, lVal *v){
	lVal *ret = lRootsValPush(lValAlloc());
	ret->type     = ltObject;
	ret->vClosure = lClosureNew(c);
	ret->vClosure->type = closureObject;
	lnfDo(ret->vClosure,v);
	return ret;
}

static lVal *lnfCurrentClosure(lClosure *c, lVal *v){
	(void)v;
	lVal *ret = lValAlloc();
	ret->type = ltObject;
	ret->vClosure = c;
	return ret;
}

static lVal *lnfCurrentLambda(lClosure *c, lVal *v){
	(void)v;
	lVal *ret = lValAlloc();
	ret->type = ltLambda;
	ret->vClosure = c;
	return ret;
}

static lVal *lnfSymbolSearch(lClosure *c, lVal *v){
	(void)c;
	const lVal *car = lCar(v);
	if((car == NULL) || (car->type != ltString)){return NULL;}
	const int len = castToInt(lCadr(v), car->vString->bufEnd - car->vString->data);
	return lSymbolSearch(car->vString->data, len);
}

void lOperationsClosure(lClosure *c){
	symType          = lSymS(":type");
	symDocumentation = lSymS(":documentation");
	symArguments     = lSymS(":arguments");
	symCode          = lSymS(":code");
	symData          = lSymS(":data");

	lAddNativeFunc(c,"resolve",        "[sym]",         "Resolve SYM until it is no longer a symbol", lnfResolve);
	lAddNativeFunc(c,"resolves?",      "[sym]",         "Check if SYM resolves to a value",           lnfResolvesPred);

	lAddNativeFunc(c,"closure",        "[clo]",         "Return a tree with data about CLO",          lnfClosure);
	lAddNativeFunc(c,"closure-parent", "[clo]",         "Return the parent of CLO",                   lnfClosureParent);
	lAddNativeFunc(c,"closure-caller", "[clo]",         "Return the caller of CLO",                   lnfClosureCaller);
	lAddNativeFunc(c,"closure!",       "[clo data]",    "Overwrite fields of CLO with DATA",          lnfClosureSet);
	lAddNativeFunc(c,"current-closure","[]",            "Return the current closure as an object",    lnfCurrentClosure);
	lAddNativeFunc(c,"current-lambda", "[]",            "Return the current closure as a lambda",     lnfCurrentLambda);

	lAddNativeFunc(c,"self",           "[n]",           "Return Nth closest object closure",          lnfClSelf);
	lAddNativeFunc(c,"symbol-search",  "[str len]",     "Return a list of all symbols starting with STR",lnfSymbolSearch);
	lAddNativeFunc(c,"symbol-count",   "[]",            "Return a count of the symbols accessible from the current closure",lnfSymCount);
	lAddNativeFunc(c,"symbol-table*",  "[]",            "Return a list of all symbols defined, accessible from the current closure",lnfSymbolTable);

	lAddSpecialForm(c,"def",           "[sym val]",     "Define a new symbol SYM and link it to value VAL", lnfDef);
	lAddSpecialForm(c,"set!",          "[s v]",         "Bind a new value v to already defined symbol s",   lnfSet);
	lAddSpecialForm(c,"let*",          "[...body]",     "Run body wihtin a new closure",  lnfLetRaw);

	lAddSpecialForm(c,"λ*",           "[name args source body]", "Create a new, raw, lambda", lnfLambdaAst);
	lAddSpecialForm(c,"μ*",           "[name args source body]", "Create a new, raw, macro",  lnfMacroAst);
	lAddSpecialForm(c,"object ω",     "[...body]",               "Create a new object",       lnfObject);
}
