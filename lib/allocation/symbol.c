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

lSymbol  lSymbolList[SYM_MAX];
uint     lSymbolActive = 0;
uint     lSymbolMax    = 0;
lSymbol *lSymbolFFree = NULL;

lSymbol *symNull,*symQuote,*symQuasiquote,*symUnquote,*symUnquoteSplicing,*symArr,*symIf,*symCond,*symDo,*symMinus,*symLambda,*symLambdAst,*symTreeNew;
lSymbol *lSymLTNil, *lSymLTNoAlloc, *lSymLTBool, *lSymLTPair, *lSymLTLambda, *lSymLTInt, *lSymLTFloat, *lSymLTVec, *lSymLTString, *lSymLTSymbol, *lSymLTNativeFunction, *lSymLTSpecialForm, *lSymLTArray, *lSymLTGUIWidget, *lSymLTObject, *lSymLTDynamic, *lSymLTMacro, *lSymLTTree, *lSymLTBytecodeOp,*lSymLTBytecodeArray;

void lSymbolInit(){
	lSymbolActive   = 0;
	lSymbolMax      = 0;
	lSymbolFFree    = NULL;

	lSymbolList[0].nextFree = NULL;
	lSymbolList[0].c[sizeof(lSymbolList[0].c) - 1] = 0xFF;
	if(lSymbolList[0].nextFree != NULL){
		fpf(stderr, "Overlapping zero byte and nextFree Pointer in symbol table, exiting immediatly\n");
		exit(123);
	}

	symNull            = RSYMP(lSymS(""));
	symQuote           = RSYMP(lSymS("quote"));
	symArr             = RSYMP(lSymS("array/new"));
	symIf              = RSYMP(lSymS("if"));
	symCond            = RSYMP(lSymS("cond"));
	symDo              = RSYMP(lSymS("do"));
	symMinus           = RSYMP(lSymS("-"));
	symLambda          = RSYMP(lSymS("λ"));
	symLambdAst        = RSYMP(lSymS("λ*"));
	symTreeNew         = RSYMP(lSymS("tree/new"));
	symQuasiquote      = RSYMP(lSymS("quasiquote"));
	symUnquote         = RSYMP(lSymS("unquote"));
	symUnquoteSplicing = RSYMP(lSymS("unquote-splicing"));

	lSymLTNil            = RSYMP(lSymS(":nil"));
	lSymLTNoAlloc        = RSYMP(lSymS(":no-alloc"));
	lSymLTBool           = RSYMP(lSymS(":bool"));
	lSymLTPair           = RSYMP(lSymS(":pair"));
	lSymLTObject         = RSYMP(lSymS(":object"));
	lSymLTLambda         = RSYMP(lSymS(":lambda"));
	lSymLTInt            = RSYMP(lSymS(":int"));
	lSymLTFloat          = RSYMP(lSymS(":float"));
	lSymLTVec            = RSYMP(lSymS(":vec"));
	lSymLTString         = RSYMP(lSymS(":string"));
	lSymLTSymbol         = RSYMP(lSymS(":symbol"));
	lSymLTNativeFunction = RSYMP(lSymS(":native-function"));
	lSymLTSpecialForm    = RSYMP(lSymS(":special-form"));
	lSymLTArray          = RSYMP(lSymS(":array"));
	lSymLTGUIWidget      = RSYMP(lSymS(":gui-widget"));
	lSymLTMacro          = RSYMP(lSymS(":macro"));
	lSymLTTree           = RSYMP(lSymS(":tree"));
	lSymLTBytecodeOp     = RSYMP(lSymS(":bytecode-op"));
	lSymLTBytecodeArray  = RSYMP(lSymS(":bytecode-array"));
}

void lSymbolFree(lSymbol *s){
	s->nextFree = lSymbolFFree;
	s->c[sizeof(s->c)-1] = 0xFF;
	lSymbolFFree = s;
	lSymbolActive--;
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
		if(strncmp(str,lSymbolList[i].c,sizeof(lSymbolList[i].c)-1)){continue;}
		if(lSymbolList[i].c[sizeof(lSymbolList[i].c)-1]){continue;}
		return &lSymbolList[i];
	}
	lSymbol *ret;
	if(lSymbolFFree){
		getFirstFree:
		ret = lSymbolFFree;
		lSymbolFFree = lSymbolFFree->nextFree;
	}else{
		if(lSymbolMax >= SYM_MAX){
			lGarbageCollect();
			if(lSymbolFFree != NULL){
				goto getFirstFree;
			} else {
				lPrintError("lSym Overflow\n");
				exit(123);
			}
		}else{
			ret = &lSymbolList[lSymbolMax++];
		}
	}
	lSymbolActive++;
	spf(ret->c, &ret->c[sizeof(ret->c)], "%s", str);
	ret->c[sizeof(ret->c)-1] = 0;
	return ret;
}

lSymbol *getTypeSymbol(const lVal* v){
	if(v == NULL){return lSymLTNil;}
	switch(v->type){
		default:            return lSymLTNil;
		case ltNoAlloc:     return lSymLTNoAlloc;
		case ltBool:        return lSymLTBool;
		case ltPair:        return lSymLTPair;
		case ltObject:      return lSymLTObject;
		case ltLambda:      return lSymLTLambda;
		case ltInt:         return lSymLTInt;
		case ltFloat:       return lSymLTFloat;
		case ltVec:         return lSymLTVec;
		case ltString:      return lSymLTString;
		case ltSymbol:      return lSymLTSymbol;
		case ltNativeFunc:  return lSymLTNativeFunction;
		case ltSpecialForm: return lSymLTSpecialForm;
		case ltArray:       return lSymLTArray;
		case ltGUIWidget:   return lSymLTGUIWidget;
		case ltMacro:       return lSymLTMacro;
		case ltTree:        return lSymLTTree;
		case ltBytecodeOp:  return lSymLTBytecodeOp;
		case ltBytecodeArr: return lSymLTBytecodeArray;
	}
}
