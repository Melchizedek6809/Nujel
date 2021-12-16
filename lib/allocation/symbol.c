/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "symbol.h"
#include "val.h"
#include "../display.h"
#include "../nujel.h"
#include "../allocation/val.h"
#include "../misc/pf.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

lSymbol lSymbolList[SYM_MAX];
uint    lSymbolActive = 0;
uint    lSymbolMax    = 0;

lSymbol *symNull,*symQuote,*symQuasiquote,*symUnquote,*symUnquoteSplicing,*symArr,*symIf,*symCond,*symDo,*symMinus,*symLambda,*symLambdAst,*symTreeNew;
lSymbol *lSymLTNil, *lSymLTNoAlloc, *lSymLTBool, *lSymLTPair, *lSymLTLambda, *lSymLTInt, *lSymLTFloat, *lSymLTVec, *lSymLTString, *lSymLTSymbol, *lSymLTNativeFunction, *lSymLTSpecialForm, *lSymLTArray, *lSymLTGUIWidget, *lSymLTObject, *lSymLTDynamic, *lSymLTMacro, *lSymLTTree;

void lSymbolInit(){
	lSymbolActive   = 0;
	lSymbolMax      = 0;

	symNull            = lSymS("");
	symQuote           = lSymS("quote");
	symArr             = lSymS("array/new");
	symIf              = lSymS("if");
	symCond            = lSymS("cond");
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
	lSymLTLambda         = lSymS(":lambda");
	lSymLTInt            = lSymS(":int");
	lSymLTFloat          = lSymS(":float");
	lSymLTVec            = lSymS(":vec");
	lSymLTString         = lSymS(":string");
	lSymLTSymbol         = lSymS(":symbol");
	lSymLTNativeFunction = lSymS(":native-function");
	lSymLTSpecialForm    = lSymS(":special-form");
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
	spf(lSymbolList[lSymbolMax].c,&lSymbolList[lSymbolMax].c[sizeof(lSymbolList[0].c)],"%s",str);
	return &lSymbolList[lSymbolMax++];
}

lSymbol *getTypeSymbol(const lVal* v){
	if(v == NULL){return lSymLTNil;}
	switch(v->type){
		default:           return lSymLTNil;
		case ltNoAlloc:    return lSymLTNoAlloc;
		case ltBool:       return lSymLTBool;
		case ltPair:       return lSymLTPair;
		case ltObject:     return lSymLTObject;
		case ltLambda:     return lSymLTLambda;
		case ltInt:        return lSymLTInt;
		case ltFloat:      return lSymLTFloat;
		case ltVec:        return lSymLTVec;
		case ltString:     return lSymLTString;
		case ltSymbol:     return lSymLTSymbol;
		case ltNativeFunc: return lSymLTNativeFunction;
		case ltSpecialForm:return lSymLTSpecialForm;
		case ltArray:      return lSymLTArray;
		case ltGUIWidget:  return lSymLTGUIWidget;
		case ltMacro:      return lSymLTMacro;
		case ltTree:       return lSymLTTree;
	}
}
