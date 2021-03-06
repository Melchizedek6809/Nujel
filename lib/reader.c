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

#define isopenparen(v)  ((v=='[')||(v=='('))
#define iscloseparen(v) ((v==']')||(v==')'))
#define isparen(v) (isopenparen(v) || (iscloseparen(v)))
#define isnonsymbol(v) (isparen(v)||(v=='#')||(v=='\'')||(v=='\"')||(v=='`')||(v==';'))
#define isnumericseparator(v) ((v=='_') || (v==','))

typedef struct {
	const char *buf, *bufEnd, *data;
	lClosure *c;
} lReadContext;

static lVal *lReadValue(lReadContext *s);
static lVal *lReadList(lReadContext *s, bool rootForm);

static NORETURN void lExceptionThrowReader(lReadContext *s, const char *msg){
	lVal *err = lValStringError(s->buf, s->bufEnd, MAX(s->buf, s->bufEnd-30) ,s->bufEnd , s->bufEnd);
	lExceptionThrowValClo("read-error", msg, err, s->c);
}

static NORETURN void lExceptionThrowReaderCustom(lReadContext *s, const char *msg, const char *customError){
	lVal *err = lValStringError(s->buf, s->bufEnd, MAX(s->buf, s->bufEnd-30) ,s->bufEnd , s->bufEnd);
	lExceptionThrowValClo(customError, msg, err, s->c);
}

static NORETURN void lExceptionThrowReaderStartEnd(lReadContext *s, const char *msg){
	const char *start, *end;
	for(start = s->data; (start > s->buf) && (*start != '"') && ((start <= s->buf) || (start[-1] != '\\')); start--){}
	for(end = s->data; (end < s->bufEnd) && (*end != '"') && (end[-1] != '\\'); end++){}
	lExceptionThrowValClo("read-error", msg, lValStringError(s->buf,s->bufEnd, start ,s->data , end), s->c);
}

static NORETURN void lExceptionThrowReaderEnd(lReadContext *s, const char *start, const char *msg){
	const char *end;
	for(end = s->data; (end < s->bufEnd) && ((*end > ' ') && !isnonsymbol(*end)); end++){}
	lExceptionThrowValClo("read-error", msg, lValStringError(s->buf,s->bufEnd, start ,s->data , end), s->c);
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
static lVal *lParseString(lReadContext *s){
	static char *buf = NULL;
	static uint bufSize = 1<<12; // Start with 4K
	if(buf == NULL){buf = malloc(bufSize);}
	if(buf == NULL){
		fpf(stderr, "Can't alloc parse buf\n");
		exit(20);
	}
	char *b = buf;
	uint i=0;
	while(s->data < s->bufEnd){
		if(unlikely(++i >= bufSize)){
			bufSize *= 2;
			buf = realloc(buf,bufSize);
			if(buf == NULL){
				fpf(stderr, "Can't grow parse buf\n");
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
				lExceptionThrowReaderStartEnd(s, "Unknown escape character");
				break;
			}
			s->data++;
		}else if(unlikely(*s->data == '"')){
			s->data++;
			lVal *v = lValAlloc(ltString);
			v->vString = lStringNew(buf,b-buf);
			return v;
		}else if(unlikely(*s->data == 0)){
			if (likely(i < bufSize)) {
				buf[i] = 0;
			}
			lExceptionThrowValClo("read-error", "Can't find closing \"", lValString(buf), s->c);
			return NULL;
		}else{
			*b++ = *s->data++;
		}
	}
	if (likely(i < bufSize)) {
		buf[i] = 0;
	}
	lExceptionThrowValClo("read-error", "Can't find closing \"", lValString(buf), s->c);
	return NULL;
}

/* Parse s as a symbol and return the ltSymbol lVal */
static lVal *lParseSymbol(lReadContext *s){
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
				lExceptionThrowReaderEnd(s, start, "Can't have a colon there");
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
	if(unlikely(*start == 0)){
		lExceptionThrowReaderEnd(s, kwstart, "Sym/KW too short");
	}
	return keyword
		? lValKeyword(kwstart)
		: lValSym(buf);
}

static i64 lParseNumberBase(lReadContext *s, int *leadingZeroes, int base, int maxDigits){
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
				lExceptionThrowReaderEnd(s, start, "Literal too big, loss of precision imminent");
			}
		}else{
			if(!isnumericseparator(c)){
				lExceptionThrowReaderEnd(s, start, "Wrong char in literal");
			}
		}
	}

	if(leadingZeroes != NULL){*leadingZeroes = zeroes;}
	return ret;
}

static lVal *lParseNumber(lReadContext *s, int base, int maxDigits){
	const char *start = s->data;
	bool negative = false;
	if(*start == '-'){
		s->data++;
		negative = true;
	}
	const i64 val = lParseNumberBase(s, NULL, base, maxDigits);
	if(*s->data == '.'){
		s->data++;
		int mantissaLeadingZeroes = 0;
		const i64 mantissaVal = lParseNumberBase(s, &mantissaLeadingZeroes, base, maxDigits);
		if(*s->data == '.'){
			lExceptionThrowReaderEnd(s, start, "Period at end of number");
		}else{
			const double valf = createFloat(val,mantissaVal, mantissaLeadingZeroes);
			return lValFloat(negative ? -valf : valf);
		}
	}
	return lValInt(negative ? -val : val);
}

static lVal *lParseCharacter(lReadContext *s){
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

static lVal *lParseBytecodeOp(lReadContext *s){
	lVal *ret = lParseNumber(s, 16, 2);
	if((ret->vInt < 0) || (ret->vInt > 255)){
		lExceptionThrowReaderStartEnd(s, "Out of bound op");
	}
	const i64 code = ret->vInt;
	ret->type = ltBytecodeOp;
	ret->vBytecodeOp = code;
	return ret;
}

static NORETURN void throwBCReadError(lReadContext *s, lVal *v, const char *msg){
	char buf[128];
	spf(buf, &buf[sizeof(buf)], "invalid %s in Bytecoded Array", msg);
	lExceptionThrowValClo("read-error", buf, lCons(v, lValStringError(s->buf,s->bufEnd, s->data ,s->data ,s->data)), s->c);
}

static lVal *lParseBytecodeArray(lReadContext *s){
	u8 *d    = NULL;
	int size = 0;
	int len  = 0;
	lArray *literals = NULL;

	if(*s->data == '#'){
		lVal *v = lReadValue(s);
		if(v->type != ltArray){
			throwBCReadError(s, v, "Invalid literal array in BCA");
		}
		literals = v->vArray;
		literals->flags = ARRAY_IMMUTABLE;
	}

	while(s->data < s->bufEnd){
		if((len+4) >= size){
			size = MAX(size,128) * 2;
			u8 *newD = realloc(d, size);
			if(unlikely(newD == NULL)){
				free(d);
				lExceptionThrowValClo("out-of-memory", "OOM during BC Arr Parse", NULL, s->c);
				return NULL;
			}
			d = newD;
		}
		lStringAdvanceToNextCharacter(s);
		char c = *s->data++;
		int t = 0;
		if((c >= '0') && (c <= '9')){t =  (c - '0')      << 4; goto readSecondNibble;}
		if((c >= 'A') && (c <= 'F')){t = ((c - 'A')+0xA) << 4; goto readSecondNibble;}
		if((c >= 'a') && (c <= 'f')){t = ((c - 'a')+0xA) << 4; goto readSecondNibble;}
		if(c == '}'){break;}

		readSecondNibble:
		if(s->data >= s->bufEnd){
			throwBCReadError(s, NULL, "sudden end");
		}
		c = *s->data++;
		if((c >= '0')  && (c <= '9')){t |=  (c - '0');      goto storeOP;}
		if((c >= 'A')  && (c <= 'F')){t |= ((c - 'A')+0xA); goto storeOP;}
		if((c >= 'a')  && (c <= 'f')){t |= ((c - 'a')+0xA); goto storeOP;}
		lExceptionThrowValClo("read-error", "Wrong char in BCArr", lValStringError(s->buf,s->bufEnd, s->data ,s->data ,s->data+1), s->c);

		storeOP:
		d[len++] = (u8)t;
	}
	lVal *ret = lValAlloc(ltBytecodeArr);
	ret->vBytecodeArr = lBytecodeArrayAlloc(len);
	ret->vBytecodeArr->literals = literals;
	if(d){
		memcpy(ret->vBytecodeArr->data, d, len);
		free(d);
	}
	return ret;
}

static u8 lReadNibble(lReadContext *s, const u8 c){
	if(c <  '0'){lExceptionThrowReaderStartEnd(s, "Wrong char in buffer lit.");}
	if(c <= '9'){
		return c - '0';
	}else{
		if((c < 'A') || (c > 'F')){
			lExceptionThrowReaderStartEnd(s, "Wrong char in buffer lit.");
		}
		return (c - 'A') + 0xA;
	}
}

static lVal *lParseBuffer(lReadContext *s){
	u8 *buf = NULL;
	size_t len = 0;
	size_t bufSize = 0;
	while(s->data < s->bufEnd){
		u8 curByte = 0;
		u8 c = *s->data;
		if(isspace(c) || isnonsymbol(c)){break;}
		curByte  = lReadNibble(s, c) << 4;

		s->data++;
		if(s->data >= s->bufEnd){
			free(buf);
			lExceptionThrowReaderStartEnd(s, "Unexpected end of buffer");
		}
		c = *s->data++;
		if(isspace(c)){
			free(buf);
			lExceptionThrowReaderStartEnd(s, "Unexpected end of literal");
		}
		curByte |= lReadNibble(s, c);

		if(len >= bufSize){
			bufSize = MAX(bufSize*2, 256);
			u8 *newBuf = realloc(buf, bufSize);
			if(unlikely(newBuf == NULL)){
				free(buf);
				lExceptionThrowValClo("out-of-memory", "OOM during buffer parse", NULL, s->c);
				return NULL;
			}
			buf = newBuf;
		}
		buf[len++] = curByte;
	}
	lStringAdvanceToNextCharacter(s);

	u8* newBuf = realloc(buf, len);
	if (unlikely(newBuf == NULL)) {
		lExceptionThrowValClo("out-of-memory", "OOM during buffer parse outtro", NULL, s->c);
		return NULL;
	}
	lVal *ret = lValAlloc(ltBuffer);
	ret->vBuffer = lBufferAlloc(len, true);
	ret->vBuffer->buf = newBuf;
	return ret;
}

static lVal *lParseSpecial(lReadContext *s){
	if(s->data >= s->bufEnd){return NULL;}
	switch(*s->data++){
	default: lExceptionThrowReaderStartEnd(s, "Wrong char in special lit.");
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
	case '$': return lParseBytecodeOp(s);
	case 'n':
		lStringAdvanceToNextSpaceOrSpecial(s);
		return NULL;
	case 't': return lValBool(true);
	case 'f': return lValBool(false);
	case '{': return lParseBytecodeArray(s);
	case '#':
		s->data++;
		return lnfArrNew(s->c, lReadList(s, false));
	case '[':
		return lCons(lValSymS(symArr), lReadList(s,false));
	case '@':{
		s->data++;
		lVal *ret = lnfTreeNew(s->c, lReadList(s,false));
		ret->vTree->flags |= TREE_IMMUTABLE;
		return ret;
	}}
}

static lVal *lReadList(lReadContext *s, bool rootForm){
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
				lExceptionThrowReaderCustom(s, "Unmatched opening bracket", "unmatched-opening-bracket");
			}
			s->data++;
			return ret == NULL ? lCons(NULL,NULL) : ret;
		}else if(iscloseparen(c)){
			if(rootForm){
				lExceptionThrowReader(s, "Unmatched closing bracket");
			}
			s->data++;
			return ret == NULL ? lCons(NULL,NULL) : ret;
		}else{
			const u8 next = s->data[1];
			if((c == '.') && (isspace(next) || isnonsymbol(next))){
				if(v == NULL){
					lExceptionThrowReader(s, "Missing car in dotted pair");
				}
				s->data++;
				lVal *nv;
				do {
					if((s->data >= s->bufEnd) || (*s->data == 0) || iscloseparen(*s->data)){
						lExceptionThrowReader(s, "Missing cdr in dotted pair");
					}
					lStringAdvanceToNextCharacter(s);
					nv = lReadValue(s);
				} while(isComment(nv));
				v->vList.cdr = isComment(nv) ? NULL : nv;
				continue;
			}else{
				lVal *nv = lReadValue(s);
				if(isComment(nv)){continue;}
				if(v == NULL){
					v = ret = lCons(nv,NULL);
				}else{
					v->vList.cdr = lCons(nv,NULL);
					v = v->vList.cdr;
				}
			}
		}
	}
}

static lVal *lReadQuote(lReadContext *s, lSymbol *carSym){
	return lCons(lValSymS(carSym), lCons(lReadValue(s),NULL));
}

static lVal *lReadValue(lReadContext *s){
	if(s->data >= s->bufEnd){
		return NULL;
	}
	const char c = *s->data;

	switch(c){
	case 0:
		return NULL;
	case '(':
	case '[':
		s->data++;
		return lReadList(s,false);
	case '~':
		s->data++;
		if(*s->data == '@'){
			s->data++;
			return lReadQuote(s, symUnquoteSplicing);
		}else{
			return lReadQuote(s, symUnquote);
		}
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
			return lCons(lValSymS(symTreeNew), lReadList(s,false));
		}
		// fall through
	default: {
		const u8 n = s->data[1];
		if((isdigit((u8)c)) || ((c == '-') && isdigit(n))){
			return lParseNumber(s, 10, 18);
		}else{
			return lParseSymbol(s);
		}
		return 0; }
	}
}

lVal *lRead(lClosure *c, const char *str){
	lReadContext ctx;
	ctx.c = c;
	ctx.buf = ctx.data = str;
	ctx.bufEnd = &str[strlen(str)];
	return lReadList(&ctx, true);
}
