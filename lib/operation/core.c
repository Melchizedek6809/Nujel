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
#include "../misc/getmsecs.h"

#include <time.h>

const lSymbol *symType;
const lSymbol *symDocumentation;
const lSymbol *symArguments;
const lSymbol *symCode;
const lSymbol *symData;

static lVal *lnfDef(lClosure *c, lVal *v){
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [def] anymore", v, c);
	return NULL;
}

static lVal *lnfSet(lClosure *c, lVal *v){
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [set!] anymore", v, c);
	return NULL;
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
	if((car == NULL)
		|| !((car->type == ltLambda)
		||   (car->type == ltObject)
		||   (car->type == ltMacro))){
		return NULL;
	}
	lTree *data = requireTree(c, lCadr(v));
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

static lVal *lnfLambdaBytecodeAst(lClosure *c, lVal *v){
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [fn*] anymore", v, c);
	return NULL;
}

/* Handler for [macro* [...args] ...body] */
static lVal *lnfMacroBytecodeAst(lClosure *c, lVal *v){
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [macro*] anymore", v, c);
	return NULL;
}

/* Handler for [ω*] */
static lVal *lnfObjectAst(lClosure *c, lVal *v){
	lExceptionThrowValClo("no-more-walking", "Can't use the old style [ω*] anymore", v, c);
	return NULL;
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

static lVal *lnfApply(lClosure *c, lVal *v){
	lVal *fun = lCar(v);
	switch(fun ? fun->type : ltNoAlloc){
	default:
		lExceptionThrowValClo("type-error", "Can't apply to that", v, c);
		return NULL;
	case ltObject:
	case ltLambda:
	case ltNativeFunc:
	case ltSpecialForm:
		return lApply(c, lCadr(v), fun, fun);
	}
}

static lVal *lnfMacroApply(lClosure *c, lVal *v){
	lVal *fun = lCar(v);
	if((fun == NULL) || (fun->type != ltMacro)){
		lExceptionThrowValClo("type-error", "Can't macro-apply to that", v, c);
		return NULL;
	}
	return lLambda(c, lCadr(v), fun);
}

static lVal *lnfCar(lClosure *c, lVal *v){
	(void)c;
	return lCaar(v);
}

static lVal *lnfCdr(lClosure *c, lVal *v){
	(void)c;
	return lCdar(v);
}

static lVal *lnfCons(lClosure *c, lVal *v){
	(void)c;
	if(lCddr(v) != NULL){
		lExceptionThrowValClo("too-many-args","Cons should only be called with 2 arguments!", v, c);
	}
	return lCons(lCar(v),lCadr(v));
}

static lVal *lnfNReverse(lClosure *c, lVal *v){
	(void)c;
	lVal *t = NULL, *l = lCar(v);
	while((l != NULL) && (l->type == ltPair)){
		lVal *next = l->vList.cdr;
		l->vList.cdr = t;
		t = l;
		l = next;
	}
	return t;
}

static lVal *lnfTime(lClosure *c, lVal *v){
	(void)c;(void)v;
	return lValInt(time(NULL));
}

static lVal *lnfTimeMsecs(lClosure *c, lVal *v){
	(void)c; (void)v;
	return lValInt(getMSecs());
}

static lVal *lnfLess(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lValGreater(lCar(v), lCadr(v)) < 0);
}

static lVal *lnfUnequal(lClosure *c, lVal *v){
	(void)c;
	return lValBool(!lValEqual(lCar(v), lCadr(v)));
}

static lVal *lnfEqual(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lValEqual(lCar(v), lCadr(v)));
}

static lVal *lnfLessEqual(lClosure *c, lVal *v){
	(void)c;
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	return lValBool(lValEqual(a,b) || (lValGreater(a, b) < 0));
}

static lVal *lnfGreater(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lValGreater(lCar(v), lCadr(v)) > 0);
}

static lVal *lnfGreaterEqual(lClosure *c, lVal *v){
	(void)c;
	lVal *a = lCar(v);
	lVal *b = lCadr(v);
	return lValBool(lValEqual(a,b) || (lValGreater(a, b) > 0));
}

static lVal *lnfNilPred(lClosure *c, lVal *v){
	(void)c;
	return lValBool(lCar(v) == NULL);
}

static lVal *lnfKeywordPred(lClosure *c, lVal *v){
	(void)c;
	lVal *car = lCar(v);
	return lValBool(car ? car->type == ltKeyword : false);
}

void lOperationsCore(lClosure *c){
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

	lAddNativeFunc(c,"symbol-count",   "[]",            "Return a count of the symbols accessible from the current closure",lnfSymCount);
	lAddNativeFunc(c,"symbol-table*",  "[]",            "Return a list of all symbols defined, accessible from the current closure",lnfSymbolTable);

	lAddSpecialForm(c,"def",           "[sym val]",     "Define a new symbol SYM and link it to value VAL", lnfDef);
	lAddSpecialForm(c,"set!",          "[s v]",         "Bind a new value v to already defined symbol s",   lnfSet);
	lAddSpecialForm(c,"let*",          "body",          "Run BODY wihtin a new closure",  lnfLetRaw);

	lAddSpecialForm(c,"macro*",       "[name args source body]", "Create a new, bytecoded, macro", lnfMacroBytecodeAst);
	lAddSpecialForm(c,"fn*",          "[name args source body]", "Create a new, bytecoded, lambda", lnfLambdaBytecodeAst);
	lAddSpecialForm(c,"ω* environment*", "[]",                  "Create a new object",       lnfObjectAst);

	lAddNativeFunc(c,"apply",       "[func list]",  "Evaluate FUNC with LIST as arguments",  lnfApply);
	lAddNativeFunc(c,"macro-apply", "[macro list]", "Evaluate MACRO with LIST as arguments", lnfMacroApply);

	lAddNativeFunc(c,"car",  "[list]",     "Returs the head of LIST",          lnfCar);
	lAddNativeFunc(c,"cdr",  "[list]",     "Return the rest of LIST",          lnfCdr);
	lAddNativeFunc(c,"cons", "[car cdr]",  "Return a new pair of CAR and CDR", lnfCons);
	lAddNativeFunc(c,"nreverse","[list]",  "Return LIST in reverse order, fast but mutates", lnfNReverse);

	lAddNativeFunc(c,"time",   "          []", "Return the current unix time",lnfTime);
	lAddNativeFunc(c,"time/milliseconds","[]", "Return monotonic msecs",lnfTimeMsecs);

	lAddNativeFunc(c,"<",        "[α β]", "Return true if α is less than β",             lnfLess);
	lAddNativeFunc(c,"<=",       "[α β]", "Return true if α is less or equal to β",      lnfLessEqual);
	lAddNativeFunc(c,"==",       "[α β]", "Return true if α is equal to β",              lnfEqual);
	lAddNativeFunc(c,"!=",       "[α β]", "Return true if α is not equal to  β",         lnfUnequal);
	lAddNativeFunc(c,">=",       "[α β]", "Return true if α is greater or equal than β", lnfGreaterEqual);
	lAddNativeFunc(c,">",        "[α β]", "Return true if α is greater than β",          lnfGreater);
	lAddNativeFunc(c,"nil?",     "[α]",   "Return true if α is #nil",                    lnfNilPred);
	lAddNativeFunc(c,"keyword?", "[α]",   "Return true if α is a keyword symbol",        lnfKeywordPred);
}
