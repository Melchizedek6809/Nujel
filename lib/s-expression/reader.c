/*
 * Wolkenwelten - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 *
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "reader.h"
#include "../exception.h"
#include "../allocation/roots.h"
#include "../allocation/string.h"
#include "../allocation/symbol.h"
#include "../allocation/val.h"
#include "../collection/list.h"
#include "../type/native-function.h"
#include "../type/symbol.h"
#include "../type/val.h"

#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define isopenparen(v) ((v=='[')||(v=='(')||(v=='{'))
#define iscloseparen(v) ((v==']')||(v==')')||(v=='}'))
#define isparen(v) (isopenparen(v) || (iscloseparen(v)))
#define isnonsymbol(v) (isparen(v)||(v=='#')||(v=='\'')||(v=='\"')||(v=='`')||(v==';'))
#define isnumericseparator(v) ((v=='_') || (v==','))

lClosure *readClosure = NULL;

static float createFloat(int value, int mantissa, int mantissaLeadingZeroes){
	if(mantissa == 0){return value;}
	const float mant = mantissa * pow(10, -(floor(log10f(mantissa)) + 1 + mantissaLeadingZeroes));
	return value + mant;
}

static void lStringAdvanceToNextCharacter(lString *s){
	for(;(*s->data != 0) && (isspace((u8)*s->data));s->data++){}
}

static void lStringAdvanceToNextSpaceOrSpecial(lString *s){
	for(;(*s->data != 0) && (!isspace((u8)*s->data));s->data++){
		const u8 c = *s->data;
		if(isnonsymbol(c)){break;}
		if(*s->data == ':'){break;}
	}
}

static void lStringAdvanceToNextLine(lString *s){
	for(;(*s->data != 0) && (*s->data != '\n');s->data++){}
}

/* Parse the string literal in s and return the resulting ltString lVal */
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
			default: {
				const char *start, *end;
				for(start = s->data; (start > s->buf) && (*start != '"') && ((start <= s->buf) || (start[-1] != '\\')); start--){}
				for(end = s->data; (end < s->bufEnd) && (*end != '"') && (end[-1] != '\\'); end++){}
				lExceptionThrowValClo(":invalid-literal", "Unknown escape character found in string literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
				break; }
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

/* Parse s as a symbol and return the ltSymbol lVal */
static lVal *lParseSymbol(lString *s){
	uint i;
	char buf[128];
	for(i=0;i<4096;i++){
		char c = *s->data++;
		if((c == 0) || isspace((u8)c) || isnonsymbol(c)){
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

/* Parse s as a binary number and return it as an ltInt lVal */
static int lParseNumberBinary(lString *s, int *leadingZeroes){
	int ret = 0;
	int zeroes = 0;
	const char *start = s->data;
	for(;(s->data < s->bufEnd);s->data++){
		const u8 c = *s->data;
		if((c <= ' ') || isnonsymbol(c)){break;}
		if(!ret && (c == '0')){zeroes++;}
		if((c == '0')  || (c == '1')){
			ret <<= 1;
			if(c == '1'){ret |= 1;}
		}else if(!isnumericseparator(c)){
			const char *end;
			for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
			lExceptionThrowValClo(":invalid-literal", "Unexpected character found in binary literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		}
	}
	if(leadingZeroes != NULL){*leadingZeroes = zeroes;}
	return ret;
}

/* Parse s as an octal number and return it as an ltInt lVal */
static int lParseNumberOctal(lString *s, int *leadingZeroes){
	int ret = 0;
	int zeroes = 0;
	const char *start = s->data;
	for(;(s->data < s->bufEnd);s->data++){
		const u8 c = *s->data;
		if((c <= ' ') || isnonsymbol(c)){break;}
		if(!ret && (c == '0')){zeroes++;}
		if((c >= '0')  && (c <= '7')){
			ret = (ret << 3) |  (c - '0');
		}else if(!isnumericseparator(c)){
			const char *end;
			for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
			lExceptionThrowValClo(":invalid-literal", "Unexpected character found in octal literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		}
	}
	if(leadingZeroes != NULL){*leadingZeroes = zeroes;}
	return ret;
}

/* Parse s as an decimal number and return it as an int */
static int lParseNumberDecimal(lString *s, int *leadingZeroes){
	int ret = 0;
	int zeroes = 0;
	const char *start = s->data;

	for(;(s->data < s->bufEnd);s->data++){
		const u8 c = *s->data;
		if((c <= ' ') || isnonsymbol(c) || (c == '.')){break;}
		if(!ret && (c == '0')){zeroes++;}
		if((c >= '0')  && (c <= '9')){
			ret = (ret * 10) + (c - '0');
		}else if(!isnumericseparator(c)){
			const char *end;
			for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
			lExceptionThrowValClo(":invalid-literal", "Unexpected character found in decimal literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		}
	}
	if(leadingZeroes != NULL){*leadingZeroes = zeroes;}
	return ret;
}

/* Parse s as a hexadecimal number and return it as an ltInt lVal */
static int lParseNumberHex(lString *s, int *leadingZeroes){
	int ret = 0;
	int zeroes = 0;
	const char *start = s->data;
	for(;s->data < s->bufEnd;s->data++){
		const u8 c = *s->data;
		if((c <= ' ') || isnonsymbol(c)){break;}
		if(!ret && (c == '0')){zeroes++;}
		if((c >= '0')  && (c <= '9')){ret = (ret << 4) |  (c - '0'); continue;}
		if((c >= 'A')  && (c <= 'F')){ret = (ret << 4) | ((c - 'A')+0xA); continue;}
		if((c >= 'a')  && (c <= 'f')){ret = (ret << 4) | ((c - 'a')+0xA); continue;}
		if(!isnumericseparator(c)){
			const char *end;
			for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
			lExceptionThrowValClo(":invalid-literal", "Unexpected character found in hex literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		}
	}
	if(leadingZeroes != NULL){*leadingZeroes = zeroes;}
	return ret;
}

/* Parse s as a decimal number and return it as an lVal */
static lVal *lParseNumber(lString *s, int (*parser)(lString *, int *)){
	const char *start = s->data;
	bool negative = false;
	if(*start == '-'){
		s->data++;
		negative = true;
	}
	const int val = parser(s, NULL);
	if(*s->data == '.'){
		s->data++;
		int mantissaLeadingZeroes = 0;
		const int mantissaVal = parser(s,&mantissaLeadingZeroes);
		if(*s->data == '.'){
			const char *end;
			for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
			lExceptionThrowValClo(":invalid-literal", "Unexpected period at end of number literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		}else{
			const float valf = createFloat(val,mantissaVal, mantissaLeadingZeroes);
			return lValFloat(negative ? -valf : valf);
		}
	}else{
		return lValInt(negative ? -val : val);
	}
}

/* Parse s as a character constant and return it's value as an ltInt lVal */
static lVal *lParseCharacter(lString *s){
	int ret = s->data[0];
	if((s->data[0] == 'B') && (s->data[1] == 'a')){ret = '\b';}
	else if((s->data[0] == 'T') && (s->data[1] == 'a')){ret = '\t';}
	else if((s->data[0] == 'L') && (s->data[1] == 'i')){ret = '\n';}
	else if((s->data[0] == 'R') && (s->data[1] == 'e')){ret = '\r';}
	else if((s->data[0] == 'l') && (s->data[1] == 'f')){ret = '\n';}
	else if((s->data[0] == 'c') && (s->data[1] == 'r')){ret = '\r';}
	lStringAdvanceToNextSpaceOrSpecial(s);
	return lValInt(ret);
}

/* Parse the special value in s starting with a # and return the resulting lVal */
static lVal *lParseSpecial(lString *s){
	if(*s->data++ != '#'){return NULL;}
	switch(*s->data++){
	default: {
		const char *start, *end;
			for(start = s->data; (start > s->buf) && (*start != '#'); start--){}
			for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
			lExceptionThrowValClo(":invalid-literal", "Unexpected character found in special literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		return NULL; }
	case '!': // Ignore Shebang's
		lStringAdvanceToNextLine(s);
		return lReadValue(s);
	case '\\':return lParseCharacter(s);
	case 'x': return lParseNumber(s,lParseNumberHex);
	case 'o': return lParseNumber(s,lParseNumberOctal);
	case 'b': return lParseNumber(s,lParseNumberBinary);
	case 'd': return lParseNumber(s,lParseNumberDecimal);
	case 'n':
		lStringAdvanceToNextSpaceOrSpecial(s);
		return NULL;
	case 't':
		return lValBool(true);
	case 'f':
		return lValBool(false);
	case '[':{
		lVal *ret = lRootsValPush(lCons(NULL,NULL));
		ret->vList.car = lValSymS(symArr);
		ret->vList.cdr = lReadList(s,false);
		return ret;
	}}

}

/* Read the string in s and parse all escape sequences */
lVal *lReadList(lString *s, bool rootForm){
	lVal *v = NULL, *ret = NULL;
	(void)rootForm;
	while(1){
		lStringAdvanceToNextCharacter(s);

		const char c = *s->data;
		if(c == ';'){
			lStringAdvanceToNextLine(s);
			continue;
		}
		if((s->data >= s->bufEnd) || (c == 0)){
			if(!rootForm){
				lExceptionThrowValClo(":unmatched-opening-bracket", "Unmatched opening bracket",NULL,readClosure);
			}
			s->data++;
			return ret == NULL ? lCons(NULL,NULL) : ret;
		}else if((c == ']') || (c == ')') || (c == '}')){
			if(rootForm){
				lExceptionThrowValClo(":unmatched-closing-bracket", "Unmatched closing bracket",NULL,readClosure);
			}
			s->data++;
			return ret == NULL ? lCons(NULL,NULL) : ret;
		}else{
			if(v == NULL){
				v = ret = lRootsValPush(lCons(NULL,NULL));
			}else{
				if((c == '.') && (isspace(s->data[1]) || isnonsymbol(s->data[1]))){
					s->data++;
					lStringAdvanceToNextCharacter(s);
					v->vList.cdr = lReadValue(s);
					continue;
				}
				v->vList.cdr = lCons(NULL,NULL);
				v = v->vList.cdr;
			}
			v->vList.car = lReadValue(s);
		}

	}
}

static lVal *lReadQuote(lString *s, lSymbol *carSym){
	lVal *ret = lRootsValPush(lCons(NULL,NULL));
	ret->vList.car = lValSymS(carSym);
	ret->vList.cdr = lCons(NULL,NULL);
	ret->vList.cdr->vList.car = lReadValue(s);
	return ret;
}

/* Read the string in s and parse all escape sequences */
lVal *lReadValue(lString *s){
	if(s->data >= s->bufEnd){
		return NULL;
	}
	const char c = *s->data;

	switch(c){
	case 0:
		return NULL;
	case '(':
	case '{':
	case '[':
		s->data++;
		return lReadList(s,false);
	case '~':
		s->data++;
		if(*s->data == '@'){
			s->data++;
			return lReadQuote(s,symUnquoteSplicing);
		}else{
			return lReadQuote(s,symUnquote);
		}
	case '`':
		s->data++;
		return lReadQuote(s,symQuasiquote);
	case '\'':
		s->data++;
		return lReadQuote(s,symQuote);
	case '"':
		s->data++;
		return lParseString(s);
	case '#':
		return lParseSpecial(s);
	case ';':
		lStringAdvanceToNextLine(s);
		return lReadValue(s);
	case '@':
		if(isopenparen(s->data[1])){
			s->data+=2;
			lVal *ret = lRootsValPush(lCons(NULL,NULL));
			ret->vList.car = lValSymS(symTreeNew);
			ret->vList.cdr = lReadList(s,false);
			return ret;
		}
		// fall through
	default: {
		const u8 n = s->data[1];
		if((isdigit((u8)c)) || ((c == '-') && isdigit(n))){
			return lParseNumber(s,lParseNumberDecimal);
		}else if((c == '-') && (n != 0) && (n != '-') && (!isspace(n)) && (!isnonsymbol(n))){
			s->data++;
			return lCons(lCons(lValSymS(symMinus),lCons(lParseSymbol(s),NULL)),NULL);
		}else{
			return lParseSymbol(s);
		}
		return 0; }
	}
}
/* Read the s-expression in str */
lVal *lRead(const char *str){
	lString *s = lRootsStringPush(lStringAlloc());
	s->buf = s->data = str;
	s->bufEnd  = &str[strlen(str)];
	lVal *ret  = lReadList(s,true);
	return ret;
}

static lVal *lnfRead(lClosure *c, lVal *v){
	(void)c;
	lVal *t = lCar(v);
	if((t == NULL) || (t->type != ltString)){return NULL;}
	lString *dup = lRootsStringPush(lStringDup(t->vString));
	readClosure = c;
	t = lReadList(dup,true);
	readClosure = NULL;
	return t;
}

/* Add all reader operators to c */
void lOperationsReader(lClosure *c){
	lAddNativeFunc(c,"read", "[str]", "Read and Parses STR as an S-Expression", lnfRead);
}
