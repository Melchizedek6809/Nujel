/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <ctype.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define isparen(v) (((v) == '(') || ((v) == ')'))
#define isbracket(v) (((v) == '[') || ((v) == ']'))
#define isbrace(v) (((v) == '{') || ((v) == '}'))
#define isClosingChar(v) (((v)==')')||((v)==']')||((v)=='}'))
#define isnonsymbol(v) (isparen(v)||isbracket(v)||isbrace(v)||((v)=='#')||((v)=='\'')||((v)=='\"')||((v)=='`')||((v)==';'))
#define isnumericseparator(v) (((v)=='_') || ((v)==','))

typedef struct {
	const char *buf, *bufEnd, *data;
} lReadContext;

static lVal lReadValue(lReadContext *s);
static lVal lReadList(lReadContext *s, bool rootForm, char terminator);

static lVal lValExceptionReaderCustom(lReadContext *s, const char *msg, const lSymbol *customError){
	lVal err = lValStringError(s->buf, s->bufEnd, MAX(s->buf, s->bufEnd-30) ,s->bufEnd , s->bufEnd);
	return lValException(customError, msg, err);
}

static lVal lValExceptionReader(lReadContext *s, const char *msg) {
	return lValExceptionReaderCustom(s,msg, lSymReadError);
}

static lVal lValExceptionReaderStartEnd(lReadContext *s, const char *msg){
	const char *start, *end;
	for(start = s->data; (start > s->buf) && (*start != '"') && ((start <= s->buf) || (start[-1] != '\\')); start--){}
	for(end = s->data; (end < s->bufEnd) && (*end != '"') && (end[-1] != '\\'); end++){}
	return lValException(lSymReadError, msg, lValStringError(s->buf,s->bufEnd, start ,s->data , end));
}

static lVal lValExceptionReaderEnd(lReadContext *s, const char *start, const char *msg){
	const char *end;
	for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
	return lValException(lSymReadError, msg, lValStringError(s->buf,s->bufEnd, start ,s->data , end));
}

static double createFloat(i64 value, i64 mantissa, i64 mantissaLeadingZeroes){
	if(mantissa == 0){return value;}
	const double mant = mantissa * pow(10, -(floor(log10(mantissa)) + 1 + mantissaLeadingZeroes));
	return value + mant;
}

static void lStringAdvanceToNextCharacter(lReadContext *s){
	for(;(s->data < s->bufEnd) && (isspace((u8)*s->data));s->data++){}
}

static void lStringAdvanceToNextSpaceOrSpecial(lReadContext *s){
	for(;(s->data < s->bufEnd) && (!isspace((u8)*s->data));s->data++){
		const u8 c = *s->data;
		if(isnonsymbol(c)){break;}
		if(*s->data == ':'){break;}
	}
}
static void lStringAdvanceToNextLine(lReadContext *s){
	for(;(s->data < s->bufEnd) && (*s->data != '\n');s->data++){}
}

static void lStringAdvanceUntilEndOfBlockComment(lReadContext *s){
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
static lVal lParseString(lReadContext *s){
	static char *buf = NULL;
	static uint bufSize = 1<<12; // Start with 4K
	if(buf == NULL){buf = malloc(bufSize);}
	if(buf == NULL){
		exit(20);
	}
	char *b = buf;
	uint i=0;
	while(s->data < s->bufEnd){
		if(unlikely(++i >= bufSize)){
			bufSize *= 2;
			buf = realloc(buf,bufSize);
			if(buf == NULL){
				exit(21);
			}
			b = &buf[i];
		}
		if(unlikely(*s->data == '\\')){
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
			case '\\':
				*b++ = '\\';
				break;
			default:
				return lValExceptionReaderStartEnd(s, "Unknown escape character");
			}
			s->data++;
		}else if(unlikely(*s->data == '"')){
			s->data++;
			return lValAlloc(ltString, lStringNew(buf,b-buf));
		}else if(unlikely(*s->data == 0)){
			if (likely(i < bufSize)) {
				buf[i] = 0;
			}
			return lValException(lSymReadError, "Can't find closing \"", lValString(buf));
		}else{
			*b++ = *s->data++;
		}
	}
	if (likely(i < bufSize)) {
		buf[i] = 0;
	}
	return lValException(lSymReadError, "Can't find closing \"", lValString(buf));
}

/* Parse s as a symbol and return the ltSymbol lVal */
static lVal lParseSymbol(lReadContext *s){
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
				return lValExceptionReaderEnd(s, start, "Can't have a colon there");
			}
		}
		if(c == '.'){
			return lValExceptionReaderEnd(s, start, "Can't have a period there");
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
	if(unlikely(i > 0) && (buf[i-1] == ':')){
		buf[i-1] = 0;
	}
	if(buf[0] == ':'){
		kwstart = &buf[1];
	}
	if(unlikely(*start == 0)){
		return lValExceptionReaderEnd(s, kwstart, "Sym/KW too short");
	}
	return keyword
		? lValKeyword(kwstart)
		: lValSym(buf);
}

static lVal lParseNumberBase(lReadContext *s, int *leadingZeroes, int base, int maxDigits){
	i64 ret = 0;
	int zeroes = 0, digits = 0;
	const char *start = s->data;

	for(;s->data < s->bufEnd;s->data++){
		const u8 c = tolower(*s->data);
		if((c <= ' ') || isnonsymbol(c) || (c == '.')){break;}

		int curDigit = -1;
		if((c >= '0') && (c <= '9')){
			curDigit = c - '0';
		}else if((c >= 'a') && (c <= 'z')){
			curDigit = (c - 'a') + 10;
		}

		if((curDigit >= 0) && (curDigit < base)){
			ret = (ret * base) + curDigit;
			if(!ret){zeroes++;}
			if((++digits - zeroes) > maxDigits){
				return lValExceptionReaderEnd(s, start, "Literal too big, loss of precision imminent");
			}
		}else{
			if(!isnumericseparator(c)){
				return lValExceptionReaderEnd(s, start, "Wrong char in literal");
			}
		}
	}

	if(leadingZeroes != NULL){*leadingZeroes = zeroes;}
	return lValInt(ret);
}

static lVal lParseNumber(lReadContext *s, int base, int maxDigits){
	const char *start = s->data;
	bool negative = false;
	if(*start == '-'){
		s->data++;
		negative = true;
	}
	lVal val = lParseNumberBase(s, NULL, base, maxDigits);
	if(unlikely(val.type == ltException)){
		return val;
	}
	if(*s->data == '.'){
		s->data++;
		int mantissaLeadingZeroes = 0;
		const lVal mantissaVal = lParseNumberBase(s, &mantissaLeadingZeroes, base, maxDigits);
		if(unlikely(mantissaVal.type == ltException)){
			return mantissaVal;
		}
		if(*s->data == '.'){
			return lValExceptionReaderEnd(s, start, "Period at end of number");
		}else{
			const double valf = createFloat(val.vInt, mantissaVal.vInt, mantissaLeadingZeroes);
			return lValFloat(negative ? -valf : valf);
		}
	}
	if(negative){
		val.vInt = -val.vInt;
	}
	return val;
}

static lVal lParseCharacter(lReadContext *s){
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

static lVal lValExceptionBCRead(lReadContext *s, lVal v, const char *msg){
	char buf[128];
	snprintf(buf, sizeof(buf), "invalid %s in Bytecoded Array", msg);
	buf[sizeof(buf)-1] = 0;
	return lValException(lSymReadError, buf, lCons(v, lValStringError(s->buf,s->bufEnd, s->data ,s->data ,s->data)));
}

static lVal lParseBytecodeArray(lReadContext *s){
	static int size = 0;
	static u8 *d    = NULL; // ToDo: make this buffer static / move to stack
	int len  = 0;
	lArray *literals = NULL;

	lVal v = lReadValue(s);
	if(unlikely(v.type != ltArray)){
		return lValExceptionBCRead(s, v, "Invalid literal array in BCA");
	}
	literals = v.vArray;
	literals->flags = ARRAY_IMMUTABLE;

	while(s->data < s->bufEnd){
		if(unlikely((len+4) >= size)){
			size = MAX(size,128) * 2;
			u8 *newD = realloc(d, size);
			if(unlikely(newD == NULL)){
				free(newD);
				return lValException(lSymOOM, "OOM during BC Arr Parse", NIL);
			}
			d = newD;
		}
		lStringAdvanceToNextCharacter(s);
		char c = toupper(*s->data++);
		int t = 0;
		if((c >= '0') && (c <= '9')){t =  (c - '0')      << 4; goto readSecondNibble;}
		if((c >= 'A') && (c <= 'F')){t = ((c - 'A')+0xA) << 4; goto readSecondNibble;}
		if(c == '}'){break;}

		readSecondNibble:
		if(unlikely(s->data >= s->bufEnd)){
			return lValExceptionBCRead(s, NIL, "sudden end");
		}
		c = toupper(*s->data++);
		if((c >= '0')  && (c <= '9')){t |=  (c - '0');      goto storeOP;}
		if((c >= 'A')  && (c <= 'F')){t |= ((c - 'A')+0xA); goto storeOP;}
		return lValException(lSymReadError, "Wrong char in BCArr", lValStringError(s->buf,s->bufEnd, s->data ,s->data ,s->data+1));

		storeOP:
		d[len++] = (u8)t;
	}
	return lValBytecodeArray(d,len,literals);
}

static lVal lParseBuffer(lReadContext *s){
	u8 *buf = NULL;
	size_t len = 0;
	size_t bufSize = 0;
	while(s->data < s->bufEnd){
		u8 curByte = 0;
		u8 c = *s->data;
		if(isspace(c) || isnonsymbol(c)){break;}
		if(c <  '0'){
			free(buf);
			return lValExceptionReaderStartEnd(s, "Wrong char in buffer lit.");
		}
		if(c <= '9'){
			curByte = (c - '0') << 4;
		}else{
			if((c < 'A') || (c > 'F')){
				free(buf);
				return lValExceptionReaderStartEnd(s, "Wrong char in buffer lit.");
			}
			curByte = ((c - 'A') + 0xA) << 4;
		}

		s->data++;
		if(s->data >= s->bufEnd){
			free(buf);
			return lValExceptionReaderStartEnd(s, "Unexpected end of buffer");
		}
		c = *s->data++;
		if(isspace(c)){
			free(buf);
			return lValExceptionReaderStartEnd(s, "Unexpected end of literal");
		}
		if(c <  '0'){
			free(buf);
			return lValExceptionReaderStartEnd(s, "Wrong char in buffer lit.");
		}
		if(c <= '9'){
			curByte |= (c - '0');
		}else{
			if((c < 'A') || (c > 'F')){
				free(buf);
				return lValExceptionReaderStartEnd(s, "Wrong char in buffer lit.");
			}
			curByte |= ((c - 'A') + 0xA);
		}

		if(len >= bufSize){
			bufSize = MAX(bufSize*2, 256);
			u8 *newBuf = realloc(buf, bufSize);
			if(unlikely(newBuf == NULL)){
				free(buf);
				return lValException(lSymOOM, "OOM during buffer parse", NIL);
			}
			buf = newBuf;
		}
		buf[len++] = curByte;
	}
	lStringAdvanceToNextCharacter(s);

	u8* newBuf = realloc(buf, len);
	if (unlikely(newBuf == NULL)) {
		free(buf);
		return lValException(lSymOOM, "OOM during buffer parse outtro", NIL);
	}
	lVal ret = lValAlloc(ltBuffer, lBufferAlloc(len, true));
	ret.vBuffer->buf = newBuf;
	return ret;
}

static lVal lParseSpecial(lReadContext *s){
	if(s->data >= s->bufEnd){return NIL;}
	switch(*s->data++){
	default:
		return lValExceptionReaderStartEnd(s, "Wrong char in special lit.");
	case '|': // SRFI-30
		lStringAdvanceUntilEndOfBlockComment(s);
		lStringAdvanceToNextCharacter(s);
		return lValComment();
	case '!': // Ignore Shebang's
		lStringAdvanceToNextLine(s);
		return lValComment();
	case '\\': return lParseCharacter(s);
	case 'm': return lParseBuffer(s);
	case 'x': return lParseNumber(s, 16, 16);
	case 'd': return lParseNumber(s, 10, 18);
	case 'o': return lParseNumber(s,  8, 21);
	case 'b': return lParseNumber(s,  2, 64);
	case 'n':
		lStringAdvanceToNextSpaceOrSpecial(s);
		return NIL;
	case 't': return lValBool(true);
	case 'f': return lValBool(false);
	case '{': return lParseBytecodeArray(s);
	case '#':
		s->data++;
		return lnfArrNew(lReadList(s, false, ')'));
	case '@':{
		s->data++;
		lVal ret = lnfTreeNew(lReadList(s,false,')'));
		if(ret.vTree->root){
			ret.vTree->root->flags |= TREE_IMMUTABLE;
		}
		return ret;
	}}
}

static lVal lReadList(lReadContext *s, bool rootForm, char terminator){
	lVal v = NIL, ret = NIL;
	while(1){
		lStringAdvanceToNextCharacter(s);

		const char c = *s->data;
		if((s->data >= s->bufEnd) || (c == 0)){
			if(unlikely(!rootForm)){
				return lValExceptionReaderCustom(s, "Unmatched opening bracket", lSymUnmatchedOpeningBracket);
			}
			s->data++;
			return ret.type == ltNil ? lCons(NIL,NIL) : ret;
		}else if(c == ';'){
			lStringAdvanceToNextLine(s);
			continue;
		}else if(c == terminator){
			if(unlikely(rootForm)){
				return lValExceptionReader(s, "Unmatched closing bracket");
			}
			s->data++;
			return ret.type == ltNil ? lCons(NIL, NIL) : ret;
		}else if(unlikely(isClosingChar(c))){
			return lValExceptionReader(s, "Unmatched closing char");
		}else{
			const u8 next = s->data[1];
			if((c == '.') && (isspace(next) || isnonsymbol(next))){
				if(unlikely(v.type == ltNil)){
					return lValExceptionReader(s, "Missing car in dotted pair");
				}
				s->data++;
				lVal nv;
				do {
					if(unlikely((s->data >= s->bufEnd) || (*s->data == 0) || (*s->data == ')'))){
						return lValExceptionReader(s, "Missing cdr in dotted pair");
					}
					lStringAdvanceToNextCharacter(s);
					nv = lReadValue(s);
					if(unlikely(nv.type == ltException)){
						return nv;
					}
				} while(isComment(nv));
				v.vList->cdr = isComment(nv) ? NIL : nv;
				continue;
			}else{
				lVal nv = lReadValue(s);
				if(unlikely(isComment(nv))){continue;}
				if(unlikely(nv.type == ltException)){
					return nv;
				}
				if(v.type == ltNil){
					v = ret = lCons(nv, NIL);
				}else{
					v.vList->cdr = lCons(nv, NIL);
					v = v.vList->cdr;
				}
			}
		}
	}
}

static lVal lReadQuote(lReadContext *s, lSymbol *carSym){
	return lCons(lValSymS(carSym), lCons(lReadValue(s), NIL));
}

static lVal lReadValue(lReadContext *s){
	if(unlikely(s->data >= s->bufEnd)){
		return NIL;
	}
	const char c = *s->data;

	switch(c){
	case 0:
		return NIL;
	case '(':
		s->data++;
		return lReadList(s,false,')');
	case '[':
		s->data++;
		return lCons(lValSymS(symArr), lReadList(s,false, ']'));
	case '{':
		s->data++;
		return lCons(lValSymS(symTreeNew), lReadList(s,false,'}'));
	case '~':
		s->data++;
		if(*s->data == '@'){
			s->data++;
			return lReadQuote(s, symUnquoteSplicing);
		}
		return lReadQuote(s, symUnquote);
	case '`':
		s->data++;
		return lReadQuote(s, symQuasiquote);
	case '\'':
		s->data++;
		return lReadQuote(s, symQuote);
	case '"':
		s->data++;
		return lParseString(s);
	case '#':
		s->data++;
		if(s->data >= s->bufEnd){
			return NIL;
		}else if(*s->data == ';'){
			++s->data;
			if((s->data < s->bufEnd) && (*s->data == '(')){
				s->data++;
				lReadList(s,false,')');
				return lValComment();
			}else{
				lReadValue(s);
				return lValComment();
			}
		}
		return lParseSpecial(s);
	case ';':
		lStringAdvanceToNextLine(s);
		return lReadValue(s);
	default:
		if((isdigit((u8)c)) || ((c == '-') && isdigit(s->data[1]))){
			return lParseNumber(s, 10, 18);
		}
		return lParseSymbol(s);
	}
}

lVal lRead(const char *str){
	lReadContext ctx;
	ctx.buf = ctx.data = str;
	ctx.bufEnd = &str[strlen(str)];
	return lReadList(&ctx, true, ')');
}
