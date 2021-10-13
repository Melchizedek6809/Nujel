/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "string.h"

#include "../nujel.h"
#include "../casting.h"
#include "array.h"
#include "closure.h"
#include "list.h"
#include "symbol.h"
#include "val.h"
#include "vec.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

lString  lStringList[STR_MAX];
uint     lStringActive = 0;
uint     lStringMax    = 1;
uint     lStringFFree  = 0;

char *ansiRS = "\033[0m";
char *ansiFG[16] = {
	"\033[0;30m",
	"\033[0;31m",
	"\033[0;32m",
	"\033[0;33m",
	"\033[0;34m",
	"\033[0;35m",
	"\033[0;36m",
	"\033[0;37m",
	"\033[1;30m",
	"\033[1;31m",
	"\033[1;32m",
	"\033[1;33m",
	"\033[1;34m",
	"\033[1;35m",
	"\033[1;36m",
	"\033[1;37m"
};

void lInitStr(){
	lStringActive = 0;
	lStringMax    = 1;
}

u32 lStringAlloc(){
	lString *ret;
	if(lStringFFree == 0){
		if(lStringMax >= STR_MAX){
			lPrintError("lString OOM ");
			return 0;
		}
		ret = &lStringList[lStringMax++];
	}else{
		ret = &lStringList[lStringFFree & STR_MASK];
		lStringFFree  = ret->nextFree;
	}
	lStringActive++;
	*ret = (lString){0};
	return ret - lStringList;
}

void lStringFree(u32 v){
	v = v & STR_MASK;
	if((v == 0) || (v > lStringMax)){return;}
	lString *s = &lStringList[v & STR_MASK];
	if((s->buf != NULL) && (s->flags & lfHeapAlloc)){
		free((void *)s->buf);
		s->buf = NULL;
	}
	lStringActive--;
	s->nextFree = lStringFFree;
	lStringFFree = v;
}

u32 lStringNew(const char *str, uint len){
	if(str == NULL){return 0;}
	const u32 i = lStringAlloc();
	if(i == 0){return 0;}
	lString *s = &lStringList[i & STR_MASK];
	if(s == NULL){return 0;}
	char *nbuf = malloc(len+1);
	if(nbuf == NULL){return 0;}
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	s->flags |= lfHeapAlloc;
	s->buf    = s->data = nbuf;
	s->bufEnd = &s->buf[len];
	return i;
}

u32 lStringDup(uint oi){
	lString *os = &lStringList[oi & STR_MASK];
	uint len = os->bufEnd - os->buf;
	const char *str = os->data;
	const u32 i = lStringAlloc();
	if(i == 0){return 0;}
	lString *s = &lStringList[i & STR_MASK];
	if(s == NULL){return 0;}
	char *nbuf = malloc(len+1);
	memcpy(nbuf,str,len);
	nbuf[len] = 0;
	s->flags |= lfHeapAlloc;
	s->buf    = s->data = nbuf;
	s->bufEnd = &s->buf[len];
	return i;
}

int lStringLength(const lString *s){
	return s->bufEnd - s->buf;
}

lVal *lValString(const char *c){
	if(c == NULL){return NULL;}
	lVal *t = lValAlloc();
	if(t == NULL){return NULL;}
	t->type = ltString;
	t->vCdr = lStringNew(c,strlen(c));
	if(t->vCdr == 0){
		lValFree(t);
		return NULL;
	}
	return t;
}
lVal *lValCString(const char *c){
	if(c == NULL){return NULL;}
	lVal *t = lValAlloc();
	if(t == NULL){return NULL;}
	t->type = ltString;
	t->vCdr = lStringAlloc();
	if(t->vCdr == 0){
		lValFree(t);
		return NULL;
	}
	lStrBuf(t)    =  lStrData(t) = c;
	lStrEnd(t)    = &lStrBuf(t)[strlen(c)];
	lStrFlags(t) |=  lfConst;
	return t;
}

int bufPrintFloat(float v, char *buf, int t, int len){
	t = snprintf(buf,len,"%.5f",v);
	for(;buf[t-1] == '0';t--){buf[t]=0;}
	if(buf[t] == '0'){buf[t] = 0;}
	if(buf[t-1] == '.'){buf[t++] = '0';}
	return t;
}

int bufWriteString(char *buf, int len, const char *data){
	u8 c;
	char *out = buf;
	char *end = &buf[len-2];
	if(buf >= end){return 0;}
	*out++ = '\"';
	while(out < end){
		switch(c = *data++){
		case 0:
			goto bufWriteStringExit;
		case '\a': // Bell
			*out++ = '\\'; *out++ = 'a';
			break;
		case '\b': // Backspace
			*out++ = '\\'; *out++ = 'b';
			break;
		case '\t': // Horiz. Tab
			*out++ = '\\'; *out++ = 't';
			break;
		case '\n': // Line Feed
			*out++ = '\\'; *out++ = 'n';
			break;
		case '\v': // Vert. Tab
			*out++ = '\\'; *out++ = 'v';
			break;
		case '\f': // Form Feed
			*out++ = '\\'; *out++ = 'f';
			break;
		case '\r': // Carriage Return
			*out++ = '\\'; *out++ = 'r';
			break;
		case '\e': // Escape
			*out++ = '\\'; *out++ = 'e';
			break;
		case '"':
			*out++ = '\\'; *out++ = '"';
			break;
		case '\'':
			*out++ = '\\'; *out++ = '\'';
			break;
		case '\\':
			*out++ = '\\'; *out++ = '\\';
			break;
		default:
			*out++ = c;
			break;
		}
	}
	bufWriteStringExit:
	*out++ = '\"';
	return out-buf;
}

char *lSIndent(char *buf, char *bufEnd, int indentLevel){
	for(int i=0;i<indentLevel;i++){
		if(buf >= bufEnd){return buf;}
		*buf++ = ' ';
	}
	return buf;
}

char *lSWriteVal(lVal *v, char *buf, char *bufEnd, int indentLevel, bool display){
	*buf = 0;
	if(v == NULL){
		int t = snprintf(buf,bufEnd-buf,"#nil");
		return t ? buf + t : buf;
	}
	char *cur = buf;
	int t = 0;
	int len = bufEnd-buf;

	switch(v->type){
	case ltNoAlloc:
		t = snprintf(buf,len,"#zzz");
		break;
	case ltBool:
		if(v->vBool){
			t = snprintf(buf,len,"#t");
		}else{
			t = snprintf(buf,len,"#f");
		}
		break;
	case ltLambda: {
		*cur++ = '[';
		int syms = 0;
		lVal *cloText = lCloSource(v->vCdr);
		forEach(n,cloText){
			if(++syms > 2){ *cur++ = '\n';}
			if(syms > 1){
				for(int i=indentLevel;i>=0;i--){*cur++=' ';}
			}
			cur = lSWriteVal(lCar(n),cur,bufEnd,indentLevel,display);
		}
		indentLevel -= 2;
		t = snprintf(cur,bufEnd-cur,"]");
		break; }
	case ltPair: {
		int indentStyle = 0;
		int oldIndent = indentLevel;
		lVal *carSym = lCar(v);
		if((carSym != NULL) && (carSym->type == ltSymbol) && (lCdr(v) != NULL)){
			lSymbol *sym = lvSym(carSym->vCdr);
			if(sym == symQuote){
				v = lCadr(v);
				*cur++ = '\'';
				cur = lSWriteVal(v,cur,bufEnd,indentLevel,display);
				goto endOfWriteVal;
			}else if(sym == symCond){
				indentStyle = 1;
				indentLevel += 6;
			}else if(sym == symWhen){
				indentStyle = 1;
				indentLevel += 6;
			}else if(sym == symUnless){
				indentStyle = 1;
				indentLevel += 8;
			}else if(sym == symIf){
				indentStyle = 1;
				indentLevel += 3;
			}else if(sym == symLambda){
				indentStyle = 1;
				indentLevel += 3;
			}else if(sym == symLambdAst){
				indentStyle = 1;
				indentLevel += 4;
			}else if(sym == symDo){
				indentStyle = 1;
				indentLevel += 3;
			}else if(sym == symLet){
				indentStyle = 1;
				indentLevel += 5;
			}
		}
		t = snprintf(cur,bufEnd-cur,"[");
		if(t > 0){cur += t;}
		for(lVal *n = v;n != NULL; n = lCdr(n)){
			if(n->type == ltPair){
				lVal *cv = lCar(n);
				if((n == v) && (cv == NULL) && (lCdr(n) == NULL)){continue;}
				cur = lSWriteVal(cv,cur,bufEnd,indentLevel,display);
				if(lCdr(n) != NULL){
					if((indentStyle == 1) && (n != v)){
						*cur++ = '\n';
						for(int i=indentLevel;i>=0;i--){*cur++=' ';}
					}else{
						*cur++ = ' ';
					}
				}
			}else{
				*cur++ = '.';
				*cur++ = ' ';
				cur = lSWriteVal(n,cur,bufEnd,indentLevel,display);
				break;
			}
		}
		t = snprintf(cur,bufEnd-cur,"]");
		indentLevel = oldIndent;
		break; }
	case ltArray: {
		t = snprintf(cur,bufEnd-cur,"#[");
		if(t > 0){cur += t;}
		if(lArrData(v) != NULL){
			const int arrLen = lArrLength(v);
			for(int i=0;i<arrLen;i++){
				cur = lSWriteVal(lValD(lArrData(v)[i]),cur,bufEnd,indentLevel,display);
				if(i < (lArrLength(v)-1)){*cur++ = ' ';}
			}
		}
		t = snprintf(cur,bufEnd-cur,"]");
		break; }
	case ltInt:
		t = snprintf(buf,len,"%i",v->vInt);
		break;
	case ltFloat:
		t = bufPrintFloat(v->vFloat,buf,t,len);
		break;
	case ltVec:
		t  = snprintf(buf,len,"[vec ");
		t += bufPrintFloat(lVecV(v->vCdr).x,&buf[t],t,len);
		buf[t++] = ' ';
		t += bufPrintFloat(lVecV(v->vCdr).y,&buf[t],t,len);
		buf[t++] = ' ';
		t += bufPrintFloat(lVecV(v->vCdr).z,&buf[t],t,len);
		t += snprintf(&buf[t],len,"]");
		break;
	case ltString:
		if(display){
			t = snprintf(buf,len,"%s",lStrData(v));
		}else{
			t = bufWriteString(buf,len,lStrData(v));
		}
		break;
	case ltSymbol:
		t = snprintf(buf,len,"%s",lvSym(v->vCdr)->c);
		break;
	case ltSpecialForm:
		t = snprintf(buf,len,"#sfo_%u",v->vCdr);
		break;
	case ltNativeFunc:
		t = snprintf(buf,len,"#cfn_%u",v->vCdr);
		break;
	case ltInf:
		t = snprintf(buf,len,"#inf");
		break;
	case ltGUIWidget:
		t = snprintf(buf,len,"#gui_%u",v->vCdr);
		break;
	}

	endOfWriteVal:
	if(t > 0){cur += t;}
	*cur = 0;
	return cur;
}
