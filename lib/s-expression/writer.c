/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "writer.h"

#include "../misc/pf.h"

#include "../allocation/array.h"
#include "../allocation/native-function.h"
#include "../allocation/symbol.h"
#include "../collection/list.h"
#include "../collection/string.h"
#include "../collection/tree.h"
#include "../type/closure.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <ctype.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char *lSWriteTreeRec(lTree *v, char *buf, char *bufEnd, bool display){
	if((v == NULL) || (v->key == NULL)){return buf;}
	char *cur = buf;
	cur = lSWriteTreeRec(v->left,cur,bufEnd,display);
	cur = spf(cur,bufEnd,"%s ",v->key->c);
	cur = lSWriteVal(v->value,cur,bufEnd,display);
	cur = spf(cur,bufEnd," ");
	cur = lSWriteTreeRec(v->right,cur,bufEnd,display);

	return cur;
}

char *lSWriteTree(lTree *v, char *buf, char *bufEnd, bool display){
	char *cur = buf;
	cur = spf(cur,bufEnd,"@[");
	char *new = lSWriteTreeRec(v,cur,bufEnd, display);
	if(new != cur){
		cur = new;
		cur[-1] = ']';
	}else if(cur < bufEnd){
		*cur++ = ']';
	}
	return cur;
}

char *lSWriteTreeDef(lTree *v, char *buf, char *bufEnd){
	if(v == NULL){return buf;}

	buf = lSWriteTreeDef(v->left, buf, bufEnd);
	buf = spf(buf,bufEnd,"[def %s ",v->key->c);
	buf = lSWriteVal(v->value, buf, bufEnd, false);
	buf = spf(buf,bufEnd,"]\n");

	return lSWriteTreeDef(v->right, buf, bufEnd);
}

char *lSWriteVal(lVal *v, char *buf, char *bufEnd, bool display){
	if(v == NULL){
		return spf(buf,bufEnd,"#nil");
	}
	char *cur = buf;

	switch(v->type){
	default:
		return buf;
	case ltNoAlloc:
		return spf(cur, bufEnd,"#zzz");
	case ltBool:
		return spf(cur, bufEnd,"%s", v->vBool ? "#t" : "#f");
	case ltObject: {
		cur = spf(cur, bufEnd, "[ω ");
		cur = lSWriteTreeDef(v->vClosure->data, cur, bufEnd);
		return spf(cur, bufEnd, "]");}
	case ltMacro:
	case ltLambda:
		if(v->vClosure && v->vClosure->name){
			return spf(cur, bufEnd, "%s", v->vClosure->name->c);
		}else{
			return spf(cur, bufEnd, "#%s_%u", v->type == ltLambda ? "λ" : "μ", lClosureID(v->vClosure));
		}
	case ltPair: {
		lVal *carSym    = lCar(v);
		if((carSym != NULL) && (carSym->type == ltSymbol) && (lCdr(v) != NULL)){
			const lSymbol *sym = carSym->vSymbol;
			if((sym == symQuote) && (lCddr(v) == NULL) && (lCadr(v) != NULL)){
				cur = spf(cur, bufEnd, "\'");
				return lSWriteVal(lCadr(v),cur,bufEnd, false);
			}
		}
		cur = spf(cur, bufEnd, "[");
		for(lVal *n = v;n != NULL; n = lCdr(n)){
			if(n->type == ltPair){
				lVal *cv = lCar(n);
				if((n == v) && (cv == NULL) && (lCdr(n) == NULL)){continue;}
				cur = lSWriteVal(cv,cur,bufEnd, false);
				if(lCdr(n) != NULL){
					*cur++ = ' ';
				}
			}else{
				cur = spf(cur, bufEnd, ". ");
				cur = lSWriteVal(n,cur,bufEnd, false);
				break;
			}
		}
		return spf(cur, bufEnd, "]");}
	case ltTree:
		return lSWriteTree(v->vTree,cur,bufEnd, false);
	case ltArray: {
		cur = spf(cur, bufEnd, "#[");
		if(v->vArray->data != NULL){
			const int arrLen = v->vArray->length;
			for(int i=0;i<arrLen;i++){
				cur = lSWriteVal(v->vArray->data[i],cur,bufEnd, false);
				if(i < (arrLen-1)){*cur++ = ' ';}
			}
		}
		return spf(cur, bufEnd, "]");}
	case ltInt:
		return spf(cur , bufEnd, "%i" ,v->vInt);
	case ltFloat:
		return spf(cur , bufEnd, "%f" ,v->vFloat);
	case ltVec:
		return spf(cur, bufEnd, "[vec %f %f %f]", v->vVec.x, v->vVec.y, v->vVec.z);
	case ltString:
		return spf(buf, bufEnd, display ? "%s" : "%S", v->vString->data);
	case ltSymbol:
		return spf(cur, bufEnd, "%s",v->vSymbol->c);
	case ltSpecialForm:
	case ltNativeFunc:
		if(v->vNFunc->name){
			return spf(cur, bufEnd, "%s",v->vNFunc->name->c);
		}else{
			return spf(cur, bufEnd, "#%s_%u",v->type == ltNativeFunc ? "nfn" : "sfo", lNFuncID(v->vNFunc));
		}
	case ltGUIWidget:
		return spf(cur, bufEnd, "#gui_%p", v->vPointer);
	}
}
