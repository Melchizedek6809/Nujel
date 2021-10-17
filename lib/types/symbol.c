/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "symbol.h"
#include "val.h"
#include "../nujel.h"

#ifndef COSMOPOLITAN_H_
	#include <stdlib.h>
	#include <stdio.h>
	#include <string.h>
#endif

lSymbol lSymbolList[SYM_MAX];
uint    lSymbolActive = 0;
uint    lSymbolMax    = 1;

lSymbol *symNull,*symQuote,*symArr,*symIf,*symCond,*symWhen,*symUnless,*symLet,*symDo,*symMinus,*symLambda,*symLambdAst;
lSymbol *lSymLTNoAlloc, *lSymLTBool, *lSymLTPair, *lSymLTLambda, *lSymLTInt, *lSymLTFloat, *lSymLTVec, *lSymLTString, *lSymLTSymbol, *lSymLTNativeFunction, *lSymLTSpecialForm, *lSymLTInfinity, *lSymLTArray, *lSymLTGUIWidget;

void lInitSymbol(){
	lSymbolActive   = 0;
	lSymbolMax      = 1;

	symNull     = lSymS("");
	symQuote    = lSymS("quote");
	symArr      = lSymS("arr");
	symIf       = lSymS("if");
	symCond     = lSymS("cond");
	symWhen     = lSymS("when");
	symUnless   = lSymS("unless");
	symLet      = lSymS("let");
	symDo       = lSymS("do");
	symMinus    = lSymS("-");
	symLambda   = lSymS("λ");
	symLambdAst = lSymS("λ*");

	lSymLTNoAlloc        = lSymS(":no-alloc");
	lSymLTBool           = lSymS(":bool");
	lSymLTPair           = lSymS(":pair");
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

lVal *lValSymS(lSymbol *s){
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
