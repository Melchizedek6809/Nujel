/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "symbol.h"
#include "val.h"
#include "../display.h"
#include "../nujel.h"
#include "../allocation/val.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

lSymbol lSymbolList[SYM_MAX];
uint    lSymbolActive = 0;
uint    lSymbolMax    = 0;

lSymbol *symNull,*symQuote,*symQuasiquote,*symUnquote,*symUnquoteSplicing,*symArr,*symIf,*symCond,*symWhen,*symUnless,*symLet,*symDo,*symMinus,*symLambda,*symLambdAst,*symTreeNew;
lSymbol *lSymLTNil, *lSymLTNoAlloc, *lSymLTBool, *lSymLTPair, *lSymLTLambda, *lSymLTInt, *lSymLTFloat, *lSymLTVec, *lSymLTString, *lSymLTSymbol, *lSymLTNativeFunction, *lSymLTSpecialForm, *lSymLTInfinity, *lSymLTArray, *lSymLTGUIWidget, *lSymLTObject, *lSymLTDynamic, *lSymLTMacro, *lSymLTTree;

void lSymbolInit(){
	lSymbolActive   = 0;
	lSymbolMax      = 0;

	symNull            = lSymS("");
	symQuote           = lSymS("quote");
	symArr             = lSymS("arr");
	symIf              = lSymS("if");
	symCond            = lSymS("cond");
	symWhen            = lSymS("when");
	symUnless          = lSymS("unless");
	symLet             = lSymS("let");
	symDo              = lSymS("do");
	symMinus           = lSymS("-");
	symLambda          = lSymS("λ");
	symLambdAst        = lSymS("λ*");
	symTreeNew         = lSymS("tree/new");
	symQuasiquote      = lSymS("quasiquote");
	symUnquote         = lSymS("unquote");
	symUnquoteSplicing = lSymS("unquote-splicing");

	lSymLTNil            = lSymS(":nil");
	lSymLTNoAlloc        = lSymS(":no-alloc");
	lSymLTBool           = lSymS(":bool");
	lSymLTPair           = lSymS(":pair");
	lSymLTObject         = lSymS(":object");
	lSymLTDynamic        = lSymS(":dynamic");
	lSymLTLambda         = lSymS(":lambda");
	lSymLTInt            = lSymS(":int");
	lSymLTFloat          = lSymS(":float");
	lSymLTVec            = lSymS(":vec");
	lSymLTString         = lSymS(":string");
	lSymLTSymbol         = lSymS(":symbol");
	lSymLTNativeFunction = lSymS(":native-function");
	lSymLTSpecialForm    = lSymS(":special-form");
	lSymLTInfinity       = lSymS(":infinity");
	lSymLTArray          = lSymS(":array");
	lSymLTGUIWidget      = lSymS(":gui-widget");
	lSymLTMacro          = lSymS(":macro");
	lSymLTTree           = lSymS(":tree");
}

lSymbol *lSymSL(const char *str, uint len){
	char buf[32];
	len = MIN(sizeof(buf)-1,len);
	memcpy(buf,str,len);
	buf[len] = 0;
	return lSymS(buf);
}

lSymbol *lSymS(const char *str){
	for(uint i = 0;i<lSymbolMax;i++){
		if(strncmp(str,lSymbolList[i].c,sizeof(lSymbolList[0].c)-1)){continue;}
		return &lSymbolList[i];
	}
	if(lSymbolMax >= SYM_MAX){
		lPrintError("lSym Overflow\n");
		return NULL;
	}
	snprintf(lSymbolList[lSymbolMax].c,sizeof(lSymbolList[0].c),"%s",str);
	return &lSymbolList[lSymbolMax++];
}

lVal *lValSymS(const lSymbol *s){
	if(s == NULL){return NULL;}
	lVal *ret = lValAlloc();
	if(ret == NULL){return NULL;}
	ret->type = ltSymbol;
	ret->vSymbol = s;
	return ret;
}

lVal *lValSym(const char *s){
	return lValSymS(lSymS(s));
}

 bool lSymVariadic(const lSymbol *s){
	const char *p = s->c;
	if((*p == '@') || (*p == '&')){p++;}
	if((*p == '@') || (*p == '&')){p++;}
	if((p[0] == '.') && (p[1] == '.') && (p[2] == '.')){
		return true;
	}
	return false;
}

bool lSymNoEval(const lSymbol *s){
	if(s->c[0] == '@'){return true;}
	if((s->c[0] == '&') && (s->c[1] == '@')){return true;}
	return false;
}

bool lSymKeyword(const lSymbol *s){
	return s->c[0] == ':';
}

int lSymCmp(const lVal *a,const lVal *b){
	if((a == NULL) || (b == NULL)){return 2;}
	if((a->type != ltSymbol) || (b->type != ltSymbol) || (a->vSymbol == NULL)){return 2;}
	return a->vSymbol == b->vSymbol ? 0 : -1;
}

int lSymEq(const lSymbol *a,const lSymbol *b){
	return a == b ? 0 : -1;
}

lSymbol *getTypeSymbol(const lVal* v){
	if(v == NULL){return lSymLTNil;}
	switch(v->type){
	default:           return lSymLTNil;
	case ltNoAlloc:    return lSymLTNoAlloc;
	case ltBool:       return lSymLTBool;
	case ltPair:       return lSymLTPair;
	case ltObject:     return lSymLTObject;
	case ltDynamic:    return lSymLTDynamic;
	case ltLambda:     return lSymLTLambda;
	case ltInt:        return lSymLTInt;
	case ltFloat:      return lSymLTFloat;
	case ltVec:        return lSymLTVec;
	case ltString:     return lSymLTString;
	case ltSymbol:     return lSymLTSymbol;
	case ltNativeFunc: return lSymLTNativeFunction;
	case ltSpecialForm:return lSymLTSpecialForm;
	case ltInf:        return lSymLTInfinity;
	case ltArray:      return lSymLTArray;
	case ltGUIWidget:  return lSymLTGUIWidget;
	case ltMacro:      return lSymLTMacro;
	case ltTree:       return lSymLTTree;
	}
}

lVal *lSymbolSearch(const char *str, uint len){
	lVal *ret,*l;
	ret = l = NULL;
	if(str == NULL){return NULL;}
	for(uint i=0;i<lSymbolMax;i++){
		if(strncmp(lSymbolList[i].c,str,len)){continue;}
		if(l == NULL){
			ret = l = lRootsValPush(lCons(NULL,NULL));
		}else{
			l = l->vList.cdr = lCons(NULL,NULL);
		}
		l->vList.car = lValSymS(&lSymbolList[i]);
	}
	return ret;
}
