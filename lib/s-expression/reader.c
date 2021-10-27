/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "reader.h"
#include "../allocation/roots.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

static void lStringAdvanceToNextCharacter(lString *s){
	for(;(*s->data != 0) && (isspace((u8)*s->data));s->data++){}
}

static void lStringAdvanceToNextSpaceOrSpecial(lString *s){
	for(;(*s->data != 0) && (!isspace((u8)*s->data));s->data++){
		if(*s->data == '['){break;}
		if(*s->data == ']'){break;}
		if(*s->data == '('){break;}
		if(*s->data == ')'){break;}
		if(*s->data == '{'){break;}
		if(*s->data == '}'){break;}
		if(*s->data == '"'){break;}
		if(*s->data == '#'){break;}
		if(*s->data == ':'){break;}
	}
}

static void lStringAdvanceToNextLine(lString *s){
	for(;(*s->data != 0) && (*s->data != '\n');s->data++){}
}

static lVal *lParseString(lString *s){
	static char *buf = NULL;
	static uint bufSize = 1<<12; // Start with 4K
	if(buf == NULL){buf = malloc(bufSize);}
	if(buf == NULL){exit(20);}
	char *b = buf;
	uint i=0;
	while(true){
		if(++i == bufSize){
			bufSize *= 2;
			buf = realloc(buf,bufSize);
			if(buf == NULL){exit(21);}
			b = &buf[i];
		}
		if(*s->data == '\\'){
			s->data++;
			switch(*s->data){
			case '0':
				*b++ = 0;
				break;
			case 'a':
				*b++ = '\a';
				break;
			case 'b':
				*b++ = '\b';
				break;
			case 't':
				*b++ = '\t';
				break;
			case 'n':
				*b++ = '\n';
				break;
			case 'v':
				*b++ = '\v';
				break;
			case 'f':
				*b++ = '\f';
				break;
			case 'r':
				*b++ = '\r';
				break;
			case 'e':
				*b++ = '\e';
				break;
			case '"':
				*b++ = '"';
				break;
			case '\'':
				*b++ = '\'';
				break;
			case '\\':
				*b++ = '\\';
				break;
			}
			s->data++;
		}else if(*s->data == '"'){
			s->data++;
			lVal *v = lValAlloc();
			v->type = ltString;
			v->vString = lStringNew(buf,b-buf);
			return v;
		}else{
			*b++ = *s->data++;
		}
	}
	return NULL;
}

static lVal *lParseNumberDecimal(lString *s){
	lVal *v      = lValInt(0);
	char c       = *s->data;
	char fc      = c;
	bool isFloat = false;
	int cval     = 0;
	int digits   = 0;
	if(fc == '-'){c = *++s->data;}
	while(!isspace((u8)c)){
		if(c == 0){break;}
		if(isdigit((int)c)){
			cval *= 10;
			cval += c - '0';
			if(++digits > 9){break;}
		}else if(c == '.'){
			isFloat = true;
			v->vInt = cval;
			cval    = 0;
			digits  = 0;
		}else if((c != ',') && (c != '_')){
			break;
		}
		c = *++s->data;
	}
	if(isFloat){
		int t     = v->vInt;
		v->type   = ltFloat;
		float tv  = cval / powf(10.f,digits);
		v->vFloat = fc == '-' ? -(t + tv) : (t + tv);
	}else{
		v->vInt   = fc == '-' ? -cval : cval;
	}
	while(c){
		if(isspace((u8)c)){break;}
		if(!isdigit((int)c) && c != '.' && c != ',' && c != '_'){break;}
		c = *++s->data;
	}
	return v;
}

static lVal *lParseSymbol(lString *s){
	uint i;
	char buf[128];
	for(i=0;i<4096;i++){
		char c = *s->data++;
		if(isspace((u8)c)  || (c == 0)   ||
			(c == '[') || (c == ']') ||
			(c == '(') || (c == ')') ||
			(c == '{') || (c == '}'))
		{
			s->data--;
			break;
		}
		if(i < sizeof(buf)){
			buf[i] = c;
		}
	}
	buf[MIN(sizeof(buf)-1,i)] = 0;
	while(isspace((u8)*s->data)){
		if(*s->data == 0){break;}
		s->data++;
	}
	return lValSym(buf);
}

static lVal *lParseNumberBinary(lString *s){
	int ret;
	for(ret = 0;;s->data++){
		if (*s->data <= ' ')                       {break;}
		if((*s->data == '[')  || (*s->data == ']')){break;}
		if((*s->data == '(')  || (*s->data == ')')){break;}
		if((*s->data == '{')  || (*s->data == '}')){break;}
		if((*s->data == '\'') || (*s->data == '"')){break;}
		if((*s->data == '#')  || (*s->data == '`')){break;}
		if((*s->data == '0')  || (*s->data == '1')){
			ret <<= 1;
			if(*s->data == '1'){ret |= 1;}
		}
	}
	return lValInt(ret);
}

static lVal *lParseNumberHex(lString *s){
	int ret;
	for(ret = 0;;s->data++){
		if (*s->data <= ' ')                       {break;}
		if((*s->data == '[')  || (*s->data == ']')){break;}
		if((*s->data == '(')  || (*s->data == ')')){break;}
		if((*s->data == '{')  || (*s->data == '}')){break;}
		if((*s->data == '\'') || (*s->data == '"')){break;}
		if((*s->data == '#')  || (*s->data == '`')){break;}
		if((*s->data >= '0')  && (*s->data <= '9')){ret = (ret << 4) |  (*s->data - '0');}
		if((*s->data >= 'A')  && (*s->data <= 'F')){ret = (ret << 4) | ((*s->data - 'A')+0xA);}
		if((*s->data >= 'a')  && (*s->data <= 'f')){ret = (ret << 4) | ((*s->data - 'a')+0xA);}
	}
	return lValInt(ret);
}

static lVal *lParseNumberOctal(lString *s){
	int ret;
	for(ret = 0;;s->data++){
		if (*s->data <= ' ')                       {break;}
		if((*s->data == '[')  || (*s->data == ']')){break;}
		if((*s->data == '(')  || (*s->data == ')')){break;}
		if((*s->data == '{')  || (*s->data == '}')){break;}
		if((*s->data == '\'') || (*s->data == '"')){break;}
		if((*s->data == '#')  |  (*s->data == '`')){break;}
		if((*s->data >= '0')  && (*s->data <= '7')){ret = (ret << 3) |  (*s->data - '0');}
	}
	return lValInt(ret);
}

static lVal *lParseCharacter(lString *s){
	int ret = s->data[0];
	if((s->data[0] == 'B') && (s->data[1] == 'a')){ret = '\b';}
	if((s->data[0] == 'T') && (s->data[1] == 'a')){ret = '\t';}
	if((s->data[0] == 'L') && (s->data[1] == 'i')){ret = '\n';}
	if((s->data[0] == 'R') && (s->data[1] == 'e')){ret = '\r';}
	if((s->data[0] == 'l') && (s->data[1] == 'f')){ret = '\n';}
	if((s->data[0] == 'c') && (s->data[1] == 'r')){ret = '\r';}
	lStringAdvanceToNextSpaceOrSpecial(s);
	return lValInt(ret);
}

static lVal *lParseSpecial(lString *s){
	if(*s->data++ != '#'){return NULL;}
	switch(*s->data++){
	default:
	case '\\':return lParseCharacter(s);
	case 'x': return lParseNumberHex(s);
	case 'o': return lParseNumberOctal(s);
	case 'b': return lParseNumberBinary(s);
	case 'd': return lParseNumberDecimal(s);
	case 'n': s->data+=2; return NULL;
	case 't':
		return lValBool(true);
	case 'f':
		return lValBool(false);
	case 'i':
		s->data+=2; return lValInf();
	case '[':
		return lCons(lValSymS(symArr),lReadString(s));
	}

}

/* Read the string in s and parse all escape sequences */
lVal *lReadString(lString *s){
	lVal *v, *ret;
	ret = v = lCons(NULL,NULL);
	lRootsValPush(ret);
	while(1){
		lStringAdvanceToNextCharacter(s);
		char c = *s->data;
		if((v == NULL) || (c == 0) || (c == ']') || (c == ')') || (c == '}') ||(s->data >= s->bufEnd)){
			s->data++;
			lRootsValPop();
			return ret;
		}

		if(lCar(v) != NULL){
			v->vList.cdr = lCons(NULL,NULL);
			v = lCdr(v);
		}

		switch(c){
		case '(':
		case '{':
		case '[':
			s->data+=1;
			v->vList.car = lReadString(s);
			break;
		case '\'':
			s->data++;
			v->vList.car = lCons(NULL,NULL);
			v->vList.car->vList.car = lValSymS(symQuote);
			v->vList.car->vList.cdr = lCons(NULL,NULL);
			if(*s->data == '['){
				s->data++;
				v->vList.car->vList.cdr->vList.car = lReadString(s);
			}else{
				v->vList.car->vList.cdr->vList.car = lParseSymbol(s);
			}
			break;
		case '"':
			s->data++;
			v->vList.car = lParseString(s);
			break;
		case '#':
			v->vList.car = lParseSpecial(s);
			break;
		case ';':
			lStringAdvanceToNextLine(s);
			break;
		case '@':
			if((s->data[1] == '[') || (s->data[1] == '(') || (s->data[1] == '{')){
				s->data+=2;
				v->vList.cdr = lReadString(s);
				v->vList.car = lValSymS(symTreeNew);
				v->vList.car = lCons(v->vList.car,v->vList.cdr);
				v->vList.cdr = NULL;
				break;
			}
			// fall through
		default:
			if((isdigit((u8)c)) || ((c == '-') && (isdigit((u8)s->data[1])))){
				v->vList.car = lParseNumberDecimal(s);
			}else if((c == '-') && (s->data[1] != '-') && (!isspace((u8)s->data[1])) && (s->data[1] != 0)){
				s->data++;
				v->vList.car = lCons(lCons(lValSymS(symMinus),lCons(lParseSymbol(s),NULL)),NULL);
			}else{
				v->vList.car = lParseSymbol(s);
			}
			break;
		}
	}
}
/* Read the s-expression in str */
lVal *lRead(const char *str){
	lString *s  = lRootsStringPush(lStringAlloc());
	s->data     = str;
	s->buf      = str;
	s->bufEnd   = &str[strlen(str)];
	lVal *ret   = lReadString(s);
	lRootsStringPop();
	return ret;
}

static lVal *lnfRead(lClosure *c, lVal *v){
	(void)c;
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	lString *dup = lRootsStringPush(lStringDup(t->vString));
	t = lReadString(dup);
	lRootsStringPop();
	if((t != NULL) && (t->type == ltPair) && (lCar(t) != NULL) && (lCdr(t) == NULL)){
		return lCar(t);
	}else{
		return t;
	}
}

/* Add all reader operators to c */
void lOperationsReader(lClosure *c){
	lAddNativeFunc(c,"read","[str]","Read and Parses STR as an S-Expression", lnfRead);
}
