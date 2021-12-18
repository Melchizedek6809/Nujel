/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "../operation.h"

#include "../allocation/roots.h"
#include "../nujel.h"
#include "../type-system.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/val.h"

lVal *lnfvInfix;

lVal *infixFunctions[32];
int infixFunctionCount = 0;

void lAddInfix(lVal *v){
	infixFunctions[infixFunctionCount++] = v;
}

lVal *lnfInfix (lClosure *c, lVal *v){
	lVal *l = NULL, *start = NULL;
	if(v == NULL){return NULL;}
	if(v->vList.cdr == NULL){return v->vList.car;}
	start = l = lRootsValPush(lCons(NULL,NULL));
	start->vList.car = lEval(c,lCar(v));
	for(lVal *cur=lCdr(v);cur != NULL;cur=lCdr(cur)){
		l->vList.cdr = lCons(NULL,NULL);
		l = l->vList.cdr;
		l->vList.car = lEval(c,lCar(cur));
	}
	for(int i=0;i<infixFunctionCount;i++){
		lVal *func;
		for(lVal *cur=start;cur != NULL;cur=lCdr(cur)){
			tryAgain: func = lCadr(cur);
			if(func == NULL){break;}
			if(func->vNFunc != infixFunctions[i]->vNFunc){continue;}
			if(func->type != infixFunctions[i]->type){continue;}
			lVal *args = cur;
			lVal *tmp = args->vList.car;
			args->vList.car = lCadr(args);
			lCdr(args)->vList.car = tmp;
			tmp = lCddr(args)->vList.cdr;
			lCddr(args)->vList.cdr = NULL;
			args->vList.car = lEval(c,args);
			args->vList.cdr = tmp;
			goto tryAgain;
		}
	}
	return lCar(start);
}

void lOperationsInfix(lClosure *c){
	lnfvInfix = lAddNativeFunc(c,"infix","[...body]", "Evaluate body as an infix expression", lnfInfix);
}
