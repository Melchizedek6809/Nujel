 /* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <float.h>
#include <limits.h>
#include <math.h>

#define BUF_SIZE 8192

const lVal *writeValStack[256];
int         writeValSP = 0;

/* Write left part, current node and then the right part of a tree */
static char *writeTreeRec(char *cur, char *bufEnd, const lTree *v, bool display){
	if((v == NULL) || (v->key == NULL)){return cur;}
	cur = writeTreeRec(cur, bufEnd, v->left, display);
	cur = spf(cur,bufEnd,display ? " %s: %V" : " %s: %v",v->key->c, v->value);
	return writeTreeRec(cur, bufEnd, v->right, display);
}

/* Write an entire Tree structure, including @[] wrapping */
static char *writeTree(char *cur, char *bufEnd, const lTree *v, bool display){
	char *openingBracket = spf(cur,bufEnd,"#@");
	cur = writeTreeRec(openingBracket, bufEnd, v, display);
	cur += (cur == openingBracket);
	if(openingBracket < bufEnd){*openingBracket = '[';}
	if(cur < bufEnd){*cur++ = ']';}
	return cur;
}

/* Write an entire array including #[] wrapper */
static char *writeArray(char *cur, char *bufEnd, const lArray *v, bool display){
	cur = spf(cur, bufEnd, "##[");
	if(v && v->data != NULL){
		for(int i=0;i<v->length;i++){
			cur = spf(cur, bufEnd, display ? "%V%s" : "%v%s", v->data[i], (i < (v->length-1)) ? " " : "");
		}
	}
	return spf(cur, bufEnd, "]");
}

/* Return character of the lowest nibble of c */
static char getHexChar(int c){
	const char v = c & 0xF;
	return (v < 0xA) ? '0' + v : 'A' + (v - 10);
}

static char *writeBuffer(char *cur, char *bufEnd, const lBuffer *v, bool display){
	if(display){
		return spf(cur, bufEnd, "#<buffer :id %i :size %x>", v - lBufferList, v->length);
	}
	cur = spf(cur, bufEnd, "#m");
	for(int i=0;i<v->length;i++){
		const u8 c = ((u8 *)v->buf)[i];
		cur = spf(cur, bufEnd, "%c%c", (i64)getHexChar(c >> 4), (i64)getHexChar(c));
	}
	return cur;
}

/* Write a bytecode array including #{} wrapper */
static char *writeBytecodeArray(char *cur, char *bufEnd, const lBytecodeArray *v){
	if((v < lBytecodeArrayList) || ((v - lBytecodeArrayList) >= (i64)lBytecodeArrayMax)){
		epf("ERROR writing BCA\n");
		return spf(cur, bufEnd, "#{ 01 }");
	}
	cur = spf(cur, bufEnd, "#{");
	cur = writeArray(cur, bufEnd, v->literals, false);
	cur = spf(cur, bufEnd, " ");
	if(v && v->data != NULL){
		int i = 0;
		for(const lBytecodeOp *c = v->data; c < v->dataEnd; c++){
			if(cur[-1] == ' '){--cur;}
			if((i++ & 0x1F) == 0){
				cur = spf(cur, bufEnd, "\n");
			}
			cur = spf(cur, bufEnd, "%c%c", (i64)getHexChar(*c >> 4), (i64)getHexChar(*c));
		}
	}
	return spf(cur, bufEnd, "\n}");
}

static char *writePair(char *cur, char *bufEnd, const lVal *v, bool display){
	const lVal *carSym = v->vList.car;
	if((carSym != NULL) && (carSym->type == ltSymbol))		{
		if((carSym->vSymbol == symQuote)
		   && (v->vList.cdr != NULL)
		   && (v->vList.cdr->type == ltPair)
		   && (v->vList.cdr->vList.cdr == NULL)
		   && (v->vList.cdr->vList.car != NULL)){
			return spf(cur, bufEnd, display ? "\'%V" : "\'%v",v->vList.cdr->vList.car);
		}
	}
	char *openingBracket = cur;
	if(v && (v->type == ltPair) && (v->vList.car == NULL) && (v->vList.cdr == NULL)){
		cur++;
	}else{
		for(const lVal *n = v;n != NULL; n = n->vList.cdr){
			if(n->type == ltPair){
				const lVal *cv = n->vList.car;
				cur = spf(cur, bufEnd, display ? " %V" : " %v", cv);
			}else{
				cur = spf(cur, bufEnd, display ? " . %V" : " . %v", n);
				break;
			}
		}
	}
	*openingBracket = '[';
	return spf(cur, bufEnd, "]");
}

/* Write boxed value V, display determines if it should be machine- or human-readable */
static char *writeVal(char *cur, char *bufEnd, const lVal *v, bool display){
	if(v == NULL){return spf(cur,bufEnd,"#nil");}
	for(int i=0;i<writeValSP;i++){
		if(writeValStack[i] != v){continue;}
		return spf(cur, bufEnd, " -+- Loop detected -+- ");
	}

	switch(v->type){
	default:
		return cur;
	case ltNoAlloc:
		return spf(cur, bufEnd,"#zzz");
	case ltBool:
		return spf(cur, bufEnd,"%s", v->vBool ? "#t" : "#f");
	case ltObject:
		if(v->vClosure->parent == NULL){
			return spf(cur, bufEnd, "#<environment :--orphan-closure-most-likely-root-->");
		}else{
			return spf(cur, bufEnd, "#<environment %x>", v->vClosure - lClosureList);
		}
	case ltMacro:
	case ltLambda: {
		const int ID = lClosureID(v->vClosure);
		if(ID == 0){
			return spf(cur, bufEnd, "root-closure");
		}else if(v->vClosure && v->vClosure->name){
			return spf(cur, bufEnd, "%s", v->vClosure->name->c);
		}else{
			return spf(cur, bufEnd, "#%s_%u", v->type == ltLambda ? "λ" : "μ", (i64)ID);
		}
	}
	case ltPair: {
		writeValStack[writeValSP++] = v;
		char *ret = writePair(cur, bufEnd, v, display);
		writeValSP--;
		return ret; }
	case ltTree: {
		writeValStack[writeValSP++] = v;
		char *ret = writeTree(cur, bufEnd, v->vTree, display);
		writeValSP--;
		return ret; }
	case ltArray: {
		writeValStack[writeValSP++] = v;
		char *ret = writeArray(cur, bufEnd, v->vArray, display);
		writeValSP--;
		return ret; }
	case ltBytecodeArr: {
		writeValStack[writeValSP++] = v;
		char *ret = writeBytecodeArray(cur, bufEnd, v->vBytecodeArr);
		writeValSP--;
		return ret; }
	case ltBytecodeOp:
		return spf(cur , bufEnd, "#$%x" , (i64)(v->vBytecodeOp & 0xFF));
	case ltInt:
		return spf(cur , bufEnd, "%i" ,v->vInt);
	case ltFloat:
		return spf(cur , bufEnd, "%f" ,v->vFloat);
	case ltString:
		return spf(cur, bufEnd, display ? "%s" : "%S", v->vString->data);
	case ltSymbol:
		return spf(cur, bufEnd, "%s",v->vSymbol->c);
	case ltKeyword:
		return spf(cur, bufEnd, ":%s",v->vSymbol->c);
	case ltBuffer:
		return writeBuffer(cur, bufEnd, v->vBuffer, display);
	case ltBufferView:
		return spf(cur, bufEnd, "#<buffer-view %x>", v->vBufferView - lBufferViewList);
	case ltFileHandle:
		return spf(cur, bufEnd, "#<file-handle %u>", (i64)fileno(v->vFileHandle));
	case ltNativeFunc:
		if(v->vNFunc->name){
			return spf(cur, bufEnd, "%s",v->vNFunc->name->c);
		}else{
			return spf(cur, bufEnd, "#%s_%u",v->type == ltNativeFunc ? "nfn" : "sfo", lNFuncID(v->vNFunc));
		}
	}
}

/* Write the string S into the buffer */
static char *writeString(char *buf, char *bufEnd, const char *s){
	if(s == NULL){return NULL;}
	while((buf < bufEnd) && *s){
		*buf++ = *s++;
	}
	return buf;
}

/* Write the string S into the buffer while escaping all characters and wrapping
 * everything in quotes */
static char *writeStringEscaped(char *buf, char *bufEnd, const char *s){
	char *cur = buf;
	if((cur+1) >= bufEnd){return buf;}
	*cur++ = '\"';
	while(cur < (bufEnd-1)){
		const u8 c = *s++;
		switch(c){
		case 0:
			goto bufWriteStringExit;
		case '\a':
			*cur++ = '\\'; *cur++ = 'a';
			break;
		case '\b':
			*cur++ = '\\'; *cur++ = 'b';
			break;
		case '\t':
			*cur++ = '\\'; *cur++ = 't';
			break;
		case '\n':
			*cur++ = '\\'; *cur++ = 'n';
			break;
		case '\v':
			*cur++ = '\\'; *cur++ = 'v';
			break;
		case '\f':
			*cur++ = '\\'; *cur++ = 'f';
			break;
		case '\r':
			*cur++ = '\\'; *cur++ = 'r';
			break;
		case 0x1B:
			*cur++ = '\\'; *cur++ = 'e';
			break;
		case '"':
			*cur++ = '\\'; *cur++ = '"';
			break;
		case '\\':
			*cur++ = '\\'; *cur++ = '\\';
			break;
		default:
			*cur++ = c;
			break;
		}
	}
	bufWriteStringExit:
	if(cur < bufEnd){*cur++ = '\"';}
	return cur;
}

static char *writeUint(char *buf, char *bufEnd, u64 v){
	if(v >= 10){ buf = writeUint(buf, bufEnd, v / 10); }
	if(buf < bufEnd){ *buf++ = '0' + (v % 10);}
	return buf;
}

static char *writeXint(char *buf, char *bufEnd, u64 v){
	if(v >= 16){ buf = writeXint(buf, bufEnd, v >> 4); }
	if(buf < bufEnd){ *buf++ = getHexChar(v); }
	return buf;
}

/* Write the integer V into BUF */
static char *writeInt(char *buf, char *bufEnd, i64 v){
	if((v < 0) && (buf < bufEnd)){
		*buf++ = '-';
		v = -v;
	}
	return writeUint(buf, bufEnd, v);
}

/* Write out the floating point number V into BUF */
static char *writeFloat(char *buf, char *bufEnd, double v){
	double fract, integer;
	fract = fabs(modf(v, &integer));
	fract = round(fract * 100000.0) / 100000.0;
	if(fract > (1.0-DBL_EPSILON)){
		fract = 0;
		integer += integer > 0 ? 1 : -1;
	}
	char *cur = buf;
	if((v < 0) && (cur < bufEnd)){*cur++ = '-';}
	cur = writeInt(cur, bufEnd, (i64)fabs(integer));
	int zeroes = 0;
	int digits = 0;
	while((modf(fract, &integer) > DBL_EPSILON) && (++digits < 7)){
		fract *= 10.;
		zeroes += (integer == 0);
	}
	zeroes = MAX(0, zeroes - 1);
	i64 ifract = round(fract);
	while(!(ifract % 10)){
		ifract /= 10;
		if(--digits < 0){break;}
	}
	if(cur < bufEnd){*cur++ = '.';}
	while((cur < bufEnd) && zeroes--){
		*cur++ = '0';
	}
	return writeInt(cur, bufEnd, ifract);
}

char *vspf(char *buf, char *bufEnd, const char *format, va_list va){
	char *cur = buf;
	(void)va;
	while((cur < bufEnd) && (*format)){
		if(*format == '%'){
			format++;
			switch(*format){
			default:
				cur = writeString(cur, bufEnd, "UNKNOWN_SEQUENCE");
				break;
			case '%':
				*cur++ = '%';
				break;
			case 'c':
				*cur++ = (char)va_arg(va, i64);
				break;
			case 'i':
				cur = writeInt(cur, bufEnd, va_arg(va, i64));
				break;
			case 'p':
			case 'x':
				cur = writeXint(cur, bufEnd, va_arg(va, u64));
				break;
			case 'f':
				cur = writeFloat(cur, bufEnd, va_arg(va, double));
				break;
			case 'u':
				cur = writeUint(cur, bufEnd, va_arg(va, u64));
				break;
			case 's':
				cur = writeString(cur, bufEnd, va_arg(va, const char *));
				break;
			case 'S':
				cur = writeStringEscaped(cur, bufEnd, va_arg(va, const char *));
				break;
			case 'v':
				cur = writeVal(cur, bufEnd, va_arg(va, const lVal *), false);
				break;
			case 'V':
				cur = writeVal(cur, bufEnd, va_arg(va, const lVal *), true);
				break;
			case 'm':
				cur = writeTree(cur, bufEnd, va_arg(va, const lTree *), false);
				break;
			}
			format++;
		}else{
			*cur++ = *format++;
		}
	}
	cur = MIN(cur, bufEnd-1);
	*cur = 0;
	return cur;
}

char *spf(char *cur, char *bufEnd, const char *format, ...){
	va_list va;
	va_start(va ,format);
	char *ret = vspf(cur, bufEnd, format, va);
	va_end(va);
	return ret;
}

void vfpf(FILE *fp, const char *format, va_list va){
	char buf[BUF_SIZE];
	char *ret = vspf(buf,buf + sizeof(buf), format, va);
	fwrite(buf, ret - buf, 1, fp);
}

void fpf(FILE *fp, const char *format, ...){
	char buf[BUF_SIZE];
	va_list va;
	va_start(va ,format);
	char *ret = vspf(buf,buf + sizeof(buf), format, va);
	va_end(va);
	fwrite(buf, ret - buf, 1, fp);
}

void epf(const char *format, ...){
	char buf[BUF_SIZE];
	va_list va;
	va_start(va,format);
	char *ret = vspf(buf,buf + sizeof(buf), format, va);
	va_end(va);
	fwrite(buf, ret - buf, 1, stderr);
}

void pf(const char *format, ...){
	char buf[BUF_SIZE];
	va_list va;
	va_start(va ,format);
	char *ret = vspf(buf,buf + sizeof(buf), format, va);
	va_end(va);
	fwrite(buf, ret - buf, 1, stdout);
}
