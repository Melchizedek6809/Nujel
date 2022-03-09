/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "reader.h"
#include "../display.h"
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

#define isopenparen(v)  ((v=='[')||(v=='(')||(v=='{'))
#define iscloseparen(v) ((v==']')||(v==')')||(v=='}'))
#define isparen(v) (isopenparen(v) || (iscloseparen(v)))
#define isnonsymbol(v) (isparen(v)||(v=='#')||(v=='\'')||(v=='\"')||(v=='`')||(v==';'))
#define isnumericseparator(v) ((v=='_') || (v==','))

lClosure *readClosure = NULL;

static double createFloat(i64 value, i64 mantissa, i64 mantissaLeadingZeroes){
	if(mantissa == 0){return value;}
	const double mant = mantissa * pow(10, -(floor(log10(mantissa)) + 1 + mantissaLeadingZeroes));
	return value + mant;
}

static void lStringAdvanceToNextCharacter(lString *s){
	for(;(s->data < s->bufEnd) && (isspace((u8)*s->data));s->data++){}
}

static void lStringAdvanceToNextSpaceOrSpecial(lString *s){
	for(;(s->data < s->bufEnd) && (!isspace((u8)*s->data));s->data++){
		const u8 c = *s->data;
		if(isnonsymbol(c)){break;}
		if(*s->data == ':'){break;}
	}
}
static void lStringAdvanceToNextLine(lString *s){
	for(;(s->data < s->bufEnd) && (*s->data != '\n');s->data++){}
}

static void lStringAdvanceUntilEndOfBlockComment(lString *s){
	const char *end = s->bufEnd-1;
	for(;(s->data < end);s->data++){
		if((s->data[0] == '#') && (s->data[1] == '|')){
			s->data+=2;
			lStringAdvanceUntilEndOfBlockComment(s);
		}else if((s->data[0] == '|') && (s->data[1] == '#')){
			s->data+=2;
			return;
		}
	}
}

/* Parse the string literal in s and return the resulting ltString lVal */
static lVal *lParseString(lString *s){
	static char *buf = NULL;
	static uint bufSize = 1<<12; // Start with 4K
	if(buf == NULL){buf = malloc(bufSize);}
	if(buf == NULL){
		lPrintError("Couldn't allocate string buffer, exiting!\n");
		exit(20);
	}
	char *b = buf;
	uint i=0;
	while(s->data < s->bufEnd){
		if(++i == bufSize){
			bufSize *= 2;
			buf = realloc(buf,bufSize);
			if(buf == NULL){
				lPrintError("Couldn't grow string buffer, exiting!\n");
				exit(21);
			}
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
				*b++ = 0x1B; // Has to be hardcoded due to OpenWatcom
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
				lExceptionThrowValClo("invalid-literal", "Unknown escape character found in string literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
				break; }
			}
			s->data++;
		}else if(*s->data == '"'){
			s->data++;
			lVal *v = lValAlloc();
			v->type = ltString;
			v->vString = lStringNew(buf,b-buf);
			return v;
		}else if(*s->data == 0){
			buf[i] = 0;
			lExceptionThrowValClo("unclosed-string-literal", "Couldn't find a closing \" for the following string literal", lValString(buf), readClosure);
		}else{
			*b++ = *s->data++;
		}
	}
	buf[i] = 0;
	lExceptionThrowValClo("unclosed-string-literal", "Couldn't find a closing \" for the following string literal", lValString(buf), readClosure);
	return NULL;
}

/* Parse s as a symbol and return the ltSymbol lVal */
static lVal *lParseSymbol(lString *s){
	uint i;
	char buf[128];
	bool keyword = false;
	const char *start = s->data;
	for(i=0;i<(sizeof(buf)-1);i++){
		const char c = *s->data++;
		if(c == ':'){
			keyword = true;
			if(i > 0){
				const char cc = *s->data++;
				if((cc == 0) || isspace((u8)cc) || isnonsymbol(cc)){
					s->data--;
					break;
				}
				const char *end;
				for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
				lExceptionThrowValClo("invalid-literal", "can't have a colon within a symbol literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
			}
		}
		if((c == 0) || isspace((u8)c) || isnonsymbol(c)){
			s->data--;
			break;
		}
		buf[i] = c;
	}
	buf[i] = 0;
	while(isspace((u8)*s->data)){
		if(*s->data == 0){break;}
		s->data++;
	}

	char *kwstart = buf;
	if((i > 0) && (buf[i-1] == ':')){
		buf[i-1] = 0;
	}
	if(buf[0] == ':'){
		kwstart = &buf[1];
	}
	if(*start == 0){
		const char *end;
		for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
		lExceptionThrowValClo("invalid-literal", "symbols/keywords need to be at least a single character long", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
	}
	return keyword
		? lValKeyword(kwstart)
		: lValSym(buf);
}

/* Parse s as a binary number and return it as an ltInt lVal */
static i64 lParseNumberBinary(lString *s, int *leadingZeroes){
	i64 ret = 0;
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
			lExceptionThrowValClo("invalid-literal", "Unexpected character found in binary literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		}
	}
	if(leadingZeroes != NULL){*leadingZeroes = zeroes;}
	return ret;
}

/* Parse s as an octal number and return it as an ltInt lVal */
static i64 lParseNumberOctal(lString *s, int *leadingZeroes){
	i64 ret = 0;
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
			lExceptionThrowValClo("invalid-literal", "Unexpected character found in octal literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		}
	}
	if(leadingZeroes != NULL){*leadingZeroes = zeroes;}
	return ret;
}

/* Parse s as an decimal number and return it as an int */
static i64 lParseNumberDecimal(lString *s, int *leadingZeroes){
	i64 ret = 0;
	int zeroes = 0;
	int digits = 0;
	const char *start = s->data;

	for(;(s->data < s->bufEnd);s->data++){
		const u8 c = *s->data;
		if((c <= ' ') || isnonsymbol(c) || (c == '.')){break;}
		if(!ret && (c == '0')){zeroes++;}
		if((c >= '0')  && (c <= '9')){
			ret = (ret * 10) + (c - '0');
			if(++digits > 18){
				const char *end;
				for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
				lExceptionThrowValClo("invalid-literal", "Decimal literal is too big to be read without a loss in precision", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
			}
		}else if(!isnumericseparator(c)){
			const char *end;
			for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
			lExceptionThrowValClo("invalid-literal", "Unexpected character found in decimal literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		}
	}
	if(leadingZeroes != NULL){*leadingZeroes = zeroes;}
	return ret;
}

/* Parse s as a hexadecimal number and return it as an ltInt lVal */
static i64 lParseNumberHex(lString *s, int *leadingZeroes){
	i64 ret = 0;
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
			lExceptionThrowValClo("invalid-literal", "Unexpected character found in hex literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		}
	}
	if(leadingZeroes != NULL){*leadingZeroes = zeroes;}
	return ret;
}

/* Parse s as a decimal number and return it as an lVal */
static lVal *lParseNumber(lString *s, i64 (*parser)(lString *, int *)){
	const char *start = s->data;
	bool negative = false;
	if(*start == '-'){
		s->data++;
		negative = true;
	}
	const i64 val = parser(s, NULL);
	if(*s->data == '.'){
		s->data++;
		int mantissaLeadingZeroes = 0;
		const i64 mantissaVal = parser(s,&mantissaLeadingZeroes);
		if(*s->data == '.'){
			const char *end;
			for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
			lExceptionThrowValClo("invalid-literal", "Unexpected period at end of number literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		}else{
			const double valf = createFloat(val,mantissaVal, mantissaLeadingZeroes);
			return lValFloat(negative ? -valf : valf);
		}
	}
	return lValInt(negative ? -val : val);
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
	s->data++;
	lStringAdvanceToNextSpaceOrSpecial(s);
	return lValInt(ret);
}

static lVal *lParseBytecodeOp(lString *s){
	lVal *ret = lParseNumber(s,lParseNumberHex);
	if((ret->vInt < 0) || (ret->vInt > 255)){
		const char *start, *end;
		for(start = s->data; (start > s->buf) && (*start != '#'); start--){}
		for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
		lExceptionThrowValClo("invalid-literal", "Out of bounds bytecode operation literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
	}
	const i64 code = ret->vInt;
	ret->type = ltBytecodeOp;
	ret->vBytecodeOp = code;
	return ret;
}

/* Parse the special value in s starting with a # and return the resulting lVal */
static lVal *lParseSpecial(lString *s){
	if(s->data >= s->bufEnd){return NULL;}
	switch(*s->data++){
	default: {
		const char *start, *end;
			for(start = s->data; (start > s->buf) && (*start != '#'); start--){}
			for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
			lExceptionThrowValClo("invalid-literal", "Unexpected character found in special literal", lValStringError(s->buf,s->bufEnd, start ,s->data , end), readClosure);
		return NULL; }
	case '|': // SRFI-30
		lStringAdvanceUntilEndOfBlockComment(s);
		lStringAdvanceToNextCharacter(s);
		return lValComment();
	case '!': // Ignore Shebang's
		lStringAdvanceToNextLine(s);
		return lValComment();
	case '\\':return lParseCharacter(s);
	case 'x': return lParseNumber(s,lParseNumberHex);
	case 'o': return lParseNumber(s,lParseNumberOctal);
	case 'b': return lParseNumber(s,lParseNumberBinary);
	case 'd': return lParseNumber(s,lParseNumberDecimal);
	case '$': return lParseBytecodeOp(s);
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
	while(1){
		lStringAdvanceToNextCharacter(s);

		const char c = *s->data;
		if(c == ';'){
			lStringAdvanceToNextLine(s);
			continue;
		}
		if((s->data >= s->bufEnd) || (c == 0)){
			if(!rootForm){
				lVal *err = lValStringError(s->buf, s->bufEnd, MAX(s->buf, s->bufEnd-30) ,s->bufEnd , s->bufEnd);
				lExceptionThrowValClo("read-error", "Unmatched opening bracket", err,readClosure);
			}
			s->data++;
			return ret == NULL ? lCons(NULL,NULL) : ret;
		}else if(iscloseparen(c)){
			if(rootForm){
				lVal *err = lValStringError(s->buf, s->bufEnd, MAX(s->buf, s->bufEnd-30) ,s->bufEnd , s->bufEnd);
				lExceptionThrowValClo("read-error", "Unmatched closing bracket", err, readClosure);
			}
			s->data++;
			return ret == NULL ? lCons(NULL,NULL) : ret;
		}else{
			const u8 next = s->data[1];
			if((c == '.') && (isspace(next) || isnonsymbol(next))){
				if(v == NULL){
					lVal *err = lValStringError(s->buf, s->bufEnd, MAX(s->buf, s->bufEnd-30) ,s->bufEnd , s->bufEnd);
					lExceptionThrowValClo("read-error", "Missing car in dotted pair", err, readClosure);
				}
				s->data++;
				lVal *nv;
				do {
					if((s->data >= s->bufEnd) || (*s->data == 0) || iscloseparen(*s->data)){
						lVal *err = lValStringError(s->buf, s->bufEnd, MAX(s->buf, s->bufEnd-30) ,s->bufEnd , s->bufEnd);
						lExceptionThrowValClo("read-error", "Missing cdr in dotted pair", err, readClosure);
					}
					lStringAdvanceToNextCharacter(s);
					nv = lReadValue(s);
				} while(isComment(nv));
				v->vList.cdr = isComment(nv) ? NULL : nv;
				continue;
			}else{
				lVal *nv = lReadValue(s);
				if(isComment(nv)){continue;}
				RVP(nv);
				if(v == NULL){
					v = ret = lRootsValPush(lCons(nv,NULL));
				}else{
					v->vList.cdr = lCons(nv,NULL);
					v = v->vList.cdr;
				}
			}
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
		s->data++;
		if(s->data >= s->bufEnd){
			return NULL;
		}else if(*s->data == ';'){
			++s->data;
			if((s->data < s->bufEnd) && isopenparen(*s->data)){
				s->data++;
				lReadList(s,false);
				return lValComment();
			}else{
				lReadValue(s);
				return lValComment();
			}
		}else{
			return lParseSpecial(s);
		}
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
		}else{
			return lParseSymbol(s);
		}
		return 0; }
	}
}
/* Read the s-expression in str */
lVal *lRead(const char *str){
	const int SP = lRootsGet();
	lString *s = lRootsStringPush(lStringAlloc());
	s->buf     = s->data = str;
	s->bufEnd  = &str[strlen(str)];
	lVal *ret  = lReadList(s,true);
	lRootsRet(SP);
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
