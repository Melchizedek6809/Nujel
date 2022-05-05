/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "../operation.h"
#include "../misc/pf.h"
#include "../allocation/symbol.h"
#include "../exception.h"
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
	if((sym == NULL) || (sym->type != ltSymbol)){
		lExceptionThrowValClo("type-error","def needs a symbol as its first argument", v, c);
		/* Never Returns */
	}
	const lSymbol *s = sym->vSymbol;

	lVal *ret = lEval(c,lCadr(v));
	lDefineClosureSym(c,s,ret);
	return ret;
}

static lVal *lnfSet(lClosure *c, lVal *v){
	lVal *sym = lCar(v);
	if((sym == NULL) || (sym->type != ltSymbol)){
		lExceptionThrowValClo("type-error","def needs a symbol as its first argument", v, c);
		/* Never Returns */
	}
	const lSymbol *s = sym->vSymbol;

	lVal *ret = lEval(c,lCadr(v));
	if(!lSetClosureSym(c,s,ret)){
		lExceptionThrowValClo("unbound-variable","set! only works with symbols that already have an associated value", lCar(v), c);
	}
	return ret;
}

static lVal *lSymTable(lClosure *c, lVal *v){
	if(c == NULL){return v;}
	lRootsValPush(v);
	lVal *l = lTreeAddKeysToList(c->data,v);
	return lSymTable(c->parent,l);
}

static lVal *lnfSymbolTable(lClosure *c, lVal *v){
	(void)v;
	lVal *l = lSymTable(c,NULL);
	for(lVal *n = l;n;n = n->vList.cdr){
		if(n->vList.car == NULL){break;}
		n->vList.car->type = ltSymbol;
	}
	return l;
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
	ret->vTree = lTreeInsert(ret->vTree, RSYMP(lSymS("type")),lRootsValPush(lValSymS(getTypeSymbol(car))));
	if((car->type == ltSpecialForm) || (car->type == ltNativeFunc)){
		lNFunc *nf = car->vNFunc;
		if(nf == NULL){return ret;}
		ret->vTree = lTreeInsert(ret->vTree, symDocumentation,nf->doc);
		ret->vTree = lTreeInsert(ret->vTree, symArguments,nf->args);
		ret->vTree = lTreeInsert(ret->vTree, RSYMP(lSymS("name")),lValSymS(nf->name));
	}else{
		lClosure *clo = car->vClosure;
		if(clo == NULL){return ret;}
		ret->vTree = lTreeInsert(ret->vTree, symDocumentation, clo->doc);
		ret->vTree = lTreeInsert(ret->vTree, symArguments, clo->args);
		ret->vTree = lTreeInsert(ret->vTree, RSYMP(lSymS("name")), RVP(lValSymS(clo->name)));
		ret->vTree = lTreeInsert(ret->vTree, RSYMP(lSymS("code")), clo->text);
		ret->vTree = lTreeInsert(ret->vTree, RSYMP(lSymS("data")), RVP(lValTree(clo->data)));
		if(clo->type == closureCall){
			ret->vTree = lTreeInsert(ret->vTree, RSYMP(lSymS("call")), lRootsValPush(lValBool(true)));
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
		lVal *ret = lValAlloc(car->vClosure->parent->type == closureObject ? ltObject : ltLambda);
		ret->vClosure = car->vClosure->parent;
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
		lVal *ret = lValAlloc(car->vClosure->caller->type == closureObject ? ltObject : ltLambda);
		ret->vClosure = car->vClosure->caller;
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
		if(newData && (newData->flags & TREE_IMMUTABLE)){
			lExceptionThrowValClo("type-error","Closures need a mutable data tree", data->value, clo);
		}else{
			clo->data = newData;
		}
	}else {
		lExceptionThrowValClo("invalid-field","Trying to set an unknown or forbidden field for a closure", lValSymS(sym), clo);
	}
	lClosureSetRec(clo,data->left);
	lClosureSetRec(clo,data->right);
}

static lVal *lnfClosureSet(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	lTree *data = castToTree(lCadr(v),NULL);
	if(data == NULL){
		lExceptionThrowValClo("type-error","expected a tree", lCadr(v), c);
	}
	if((car == NULL)
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
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [let] anymore", v, c);
	return NULL;
}

static lVal *lnfResolve(lClosure *c, lVal *v){
	const lSymbol *sym = castToSymbol(lCar(v),NULL);
	lVal *env = lCadr(v);
	if(env && (env->type != ltLambda) && (env->type != ltObject)){
		lExceptionThrowValClo("invalid-environment", "You can only resolve symbols in Lambdas or Objects", env, c);
	}
	return sym ? lGetClosureSym(env ? env->vClosure : c, sym) : NULL;
}

static lVal *lnfResolvesPred(lClosure *c, lVal *v){
	const lSymbol *sym = castToSymbol(lCar(v),NULL);
	lVal *env = lCadr(v);
	if(env && (env->type != ltLambda) && (env->type != ltObject)){
		lExceptionThrowValClo("invalid-environment", "You can only resolve symbols in Lambdas or Objects", env, c);
	}
	return lValBool(sym ? lHasClosureSym(env ? env->vClosure : c, sym,NULL) : false);
}

/* Handler for [λδ name [..args] docstring body] */
static lVal *lnfLambdaBytecodeAst(lClosure *c, lVal *v){
	lVal *ret = lLambdaNew(c, lCar(v), lCadr(v), lCaddr(v), lCadddr(v));
	ret->vClosure->type = closureUnlinkedBytecode;
	return ret;
}

/* Handler for [macro* [...args] ...body] */
static lVal *lnfMacroBytecodeAst(lClosure *c, lVal *v){
	lVal *ret = lnfLambdaBytecodeAst(c,v);
	if(ret){ ret->type = ltMacro; }
	return ret;
}

/* Handler for [ω*] */
static lVal *lnfObjectAst(lClosure *c, lVal *v){
	(void)v;
	lVal *ret = lRootsValPush(lValAlloc(ltObject));
	ret->vClosure = lClosureNew(c, closureObject);
	return ret;
}

static lVal *lnfCurrentClosure(lClosure *c, lVal *v){
	(void)v;
	lVal *ret = lValAlloc(ltObject);
	ret->vClosure = c;
	return ret;
}

static lVal *lnfCurrentLambda(lClosure *c, lVal *v){
	(void)v;
	lVal *ret = lValAlloc(ltLambda);
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
	symType          = RSYMP(lSymS("type"));
	symDocumentation = RSYMP(lSymS("documentation"));
	symArguments     = RSYMP(lSymS("arguments"));
	symCode          = RSYMP(lSymS("code"));
	symData          = RSYMP(lSymS("data"));

	lAddNativeFunc(c,"resolve",        "[sym environment]", "Resolve SYM until it is no longer a symbol", lnfResolve);
	lAddNativeFunc(c,"resolves?",      "[sym environment]", "Check if SYM resolves to a value",           lnfResolvesPred);

	lAddNativeFunc(c,"closure",        "[clo]",         "Return a tree with data about CLO",          lnfClosure);
	lAddNativeFunc(c,"closure/parent", "[clo]",         "Return the parent of CLO",                   lnfClosureParent);
	lAddNativeFunc(c,"closure/caller", "[clo]",         "Return the caller of CLO",                   lnfClosureCaller);
	lAddNativeFunc(c,"closure!",       "[clo data]",    "Overwrite fields of CLO with DATA",          lnfClosureSet);
	lAddNativeFunc(c,"current-closure","[]",            "Return the current closure as an object",    lnfCurrentClosure);
	lAddNativeFunc(c,"current-lambda", "[]",            "Return the current closure as a lambda",     lnfCurrentLambda);

	lAddNativeFunc(c,"symbol-search",  "[str len]",     "Return a list of all symbols starting with STR",lnfSymbolSearch);
	lAddNativeFunc(c,"symbol-count",   "[]",            "Return a count of the symbols accessible from the current closure",lnfSymCount);
	lAddNativeFunc(c,"symbol-table*",  "[]",            "Return a list of all symbols defined, accessible from the current closure",lnfSymbolTable);

	lAddSpecialForm(c,"def",           "[sym val]",     "Define a new symbol SYM and link it to value VAL", lnfDef);
	lAddSpecialForm(c,"set!",          "[s v]",         "Bind a new value v to already defined symbol s",   lnfSet);
	lAddSpecialForm(c,"let*",          "body",          "Run BODY wihtin a new closure",  lnfLetRaw);

	lAddSpecialForm(c,"macro*",       "[name args source body]", "Create a new, bytecoded, macro", lnfMacroBytecodeAst);
	lAddSpecialForm(c,"fn*",          "[name args source body]", "Create a new, bytecoded, lambda", lnfLambdaBytecodeAst);
	lAddSpecialForm(c,"ω* environment*", "[]",                  "Create a new object",       lnfObjectAst);
}
