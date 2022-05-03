/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "pf.h"

#include "../allocation/closure.h"
#include "../allocation/native-function.h"
#include "../allocation/symbol.h"
#include "../collection/list.h"
#include "../type/bytecode.h"

#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>

const lVal *writeValStack[256];
int         writeValSP = 0;

static char *writeVal(char *buf, char *bufEnd, const lVal *v, bool display);

/* Write left part, current node and then the right part of a tree */
static char *writeTreeRec(char *cur, char *bufEnd, const lTree *v){
	if((v == NULL) || (v->key == NULL)){return cur;}

	cur = writeTreeRec(cur, bufEnd, v->left);
	cur = spf(cur,bufEnd,"%s: %v ",v->key->c, v->value);
	cur = writeTreeRec(cur, bufEnd, v->right);

	return cur;
}

/* Write an entire Tree structure, including @[] wrapping */
static char *writeTree(char *cur, char *bufEnd, const lTree *v){
	cur = spf(cur,bufEnd,"#@[");
	char *new = writeTreeRec(cur, bufEnd, v);
	if(new != cur){
		cur = new;
		cur[-1] = ']';
	}else if(cur < bufEnd){
		*cur++ = ']';
	}
	return cur;
}

/* Write a tree as a list of value definitions */
static char *writeTreeDef(char *cur, char *bufEnd, const lTree *v){
	if(v == NULL){return cur;}

	cur = writeTreeDef(cur, bufEnd, v->left);
	cur = spf(cur,bufEnd,"[def %s %v]\n",v->key->c, v->value);
	return writeTreeDef(cur, bufEnd, v->right);
}

/* Write an entire array including #[] wrapper */
static char *writeArray(char *cur, char *bufEnd, const lArray *v){
	cur = spf(cur, bufEnd, "##[");
	if(v && v->data != NULL){
		for(int i=0;i<v->length;i++){
			cur = spf(cur, bufEnd, "%v%s", v->data[i], (i < (v->length-1)) ? " " : "");
		}
	}
	return spf(cur, bufEnd, "]");
}

/* Return character of the lowest nibble of c */
static char getHexChar(int c){
	c &= 0xF;
	return (c < 0xA) ? '0' + c : 'A' + (c - 10);
}

static char *writeBytecodeArrayValue(char *cur, char *bufEnd, i64 index){
	if((index < 0) || (index > lValMax)){
		return spf(cur, bufEnd, "v :INVALID-VALUE ");
	}else{
		return spf(cur, bufEnd, "v %v ", lIndexVal(index));
	}
}

static char *writeBytecodeArraySymbol(char *cur, char *bufEnd, i64 index){
	if((index < 0) || (index >= lSymbolMax)){
		return spf(cur, bufEnd, "s --INVALID-SYMBOL-- ");
	}else{
		return spf(cur, bufEnd, "s %s ", lIndexSym(index)->c);
	}
}

static char *writeBytecodeArrayOffset(char *cur, char *bufEnd, i64 offset){
	if((offset > SHRT_MAX) || (offset < SHRT_MIN)){
		return spf(cur, bufEnd, "o --INVALID-OFFSET--", offset);
	}else{
		return spf(cur, bufEnd, "o %i ", offset);
	}
}

/* Write a bytecode array including #{} wrapper */
static char *writeBytecodeArray(char *cur, char *bufEnd, const lBytecodeArray *v){
	cur = spf(cur, bufEnd, "#{");
	if(v && v->data != NULL){
		for(const lBytecodeOp *c = v->data; c < v->dataEnd; c++){
			if(cur[-1] == ' '){--cur;}
			cur = spf(cur, bufEnd, "\n%c%c", (i64)getHexChar(*c >> 4), (i64)getHexChar(*c));
			switch(*c){
			case lopTry:
			case lopJt:
			case lopJf:
			case lopJmp: {
				const int off = ((i16)((c[1] << 8) | c[2]));
				cur = writeBytecodeArrayOffset(cur, bufEnd, off);
				c+=2;
				break;}
			case lopPushSymbol:
			case lopDef:
			case lopGet:
			case lopSet: {
				if(&c[3] < v->dataEnd){
					const i64 i = (c[1] << 16) | (c[2] << 8) | c[3];
					cur = writeBytecodeArraySymbol(cur, bufEnd, i);
				}
				c+=3;
				break; }
			case lopApplyDynamic: {
				if(&c[1] < v->dataEnd){
					cur = spf(cur, bufEnd, "i %i ", (i64)c[1]);
				}
				c++;
				break; }
			case lopApply: {
				if(&c[4] < v->dataEnd){
					cur = spf(cur, bufEnd, "i %i ", (i64)c[1]);
					const i64 i = ((c[2] << 16) | (c[3] << 8) | c[4]);
					cur = writeBytecodeArrayValue(cur, bufEnd, i);
				}
				c+=4;
				break; }
			case lopFn:
			case lopMacroAst: {
				if(&c[12] >= v->dataEnd){
					c+=12;
					break;
				}
				for(int i=0;i<4;i++){
					const i64 val = (c[1] << 16) | (c[2] << 8) | c[3];
					cur = writeBytecodeArrayValue(cur, bufEnd, val);
					c+=3;
				}
				break;}
			case lopIntByte: {
				if(&c[1] < v->dataEnd){
					cur = spf(cur, bufEnd, "i %i ", (i64)((i8)c[1]));
				}
				c++;
				break;}
			case lopPushLVal: {
				if(&c[3] < v->dataEnd){
					const i64 i = (c[1] << 16) | (c[2] << 8) | c[3];
					cur = writeBytecodeArrayValue(cur, bufEnd, i);
				}
				c+=3;
				break;}
			}
		}
	}
	return spf(cur, bufEnd, "\n}");
}

/* Write pair/list V, including dotted pair notation */
static char *writePair(char *cur, char *bufEnd, const lVal *v){
	const lVal *carSym = v->vList.car;
	if((carSym != NULL) && (carSym->type == ltSymbol) && (v->vList.cdr != NULL)){
		if((carSym->vSymbol == symQuote) && (v->vList.cdr != NULL) && (v->vList.cdr->type == ltPair) && (v->vList.cdr->vList.cdr == NULL) && (v->vList.cdr->vList.car != NULL)){
			return spf(cur, bufEnd, "\'%v",v->vList.cdr->vList.car);
		}
	}
	cur = spf(cur, bufEnd, "[");
	for(const lVal *n = v;n != NULL; n = n->vList.cdr){
		if(n->type == ltPair){
			const lVal *cv = n->vList.car;
			if((n == v) && (cv == NULL) && (n->vList.cdr == NULL)){continue;}
			cur = spf(cur, bufEnd, "%v%s", cv, n->vList.cdr != NULL ? " " : "");
		}else{
			cur = spf(cur, bufEnd, ". %v", n);
			break;
		}
	}
	return spf(cur, bufEnd, "]");
}

/* Write boxed value V, display determines if it should be machine- or human-readable */
static char *writeVal(char *buf, char *bufEnd, const lVal *v, bool display){
	char *cur = buf;
	char *ret = buf;

	if(v == NULL){return spf(buf,bufEnd,"#nil");}
	for(int i=0;i<writeValSP;i++){
		if(writeValStack[i] != v){continue;}
		return spf(cur, bufEnd, " -+- Loop detected -+- ");
	}
	writeValStack[writeValSP++] = v;

	switch(v->type){
	default:
		break;
	case ltNoAlloc:
		ret = spf(cur, bufEnd,"#zzz");
		break;
	case ltBool:
		ret = spf(cur, bufEnd,"%s", v->vBool ? "#t" : "#f");
		break;
	case ltObject:
		if(v->vClosure->parent == NULL){
			ret = spf(cur, bufEnd, "[ω :--orphan-closure-most-likely-root--]");
		}else{
			ret = spf(cur, bufEnd, "[ω %M]", v->vClosure->data);
		}
		break;
	case ltMacro:
	case ltLambda:
		if(v->vClosure && v->vClosure->name){
			ret = spf(cur, bufEnd, "%s", v->vClosure->name->c);
		}else{
			ret = spf(cur, bufEnd, "#%s_%u", v->type == ltLambda ? "λ" : "μ", (i64)lClosureID(v->vClosure));
		}
		break;
	case ltPair:
		ret = writePair(cur, bufEnd, v);
		break;
	case ltTree:
		ret = writeTree(cur, bufEnd, v->vTree);
		break;
	case ltArray:
		ret = writeArray(cur, bufEnd, v->vArray);
		break;
	case ltBytecodeArr:
		ret = writeBytecodeArray(cur, bufEnd, &v->vBytecodeArr);
		break;
	case ltBytecodeOp:
		ret = spf(cur , bufEnd, "#$%x" , (i64)(v->vBytecodeOp & 0xFF));
		break;
	case ltInt:
		ret = spf(cur , bufEnd, "%i" ,v->vInt);
		break;
	case ltFloat:
		ret = spf(cur , bufEnd, "%f" ,v->vFloat);
		break;
	case ltVec:
		ret = spf(cur, bufEnd, "#v[%f %f %f]", v->vVec.x, v->vVec.y, v->vVec.z);
		break;
	case ltString:
		ret = spf(buf, bufEnd, display ? "%s" : "%S", v->vString->data);
		break;
	case ltSymbol:
		ret = spf(cur, bufEnd, "%s",v->vSymbol->c);
		break;
	case ltKeyword:
		ret = spf(cur, bufEnd, ":%s",v->vSymbol->c);
		break;
	case ltSpecialForm:
	case ltNativeFunc:
		if(v->vNFunc->name){
			ret = spf(cur, bufEnd, "%s",v->vNFunc->name->c);
		}else{
			ret = spf(cur, bufEnd, "#%s_%u",v->type == ltNativeFunc ? "nfn" : "sfo", lNFuncID(v->vNFunc));
		}
		break;
	case ltGUIWidget:
		ret = spf(cur, bufEnd, "#gui_%p", v->vPointer);
		break;
	}

	writeValSP--;
	return ret;
}

/* Write the string S into the buffer */
static char *writeString(char *buf, char *bufEnd, const char *s){
	char *cur = buf;
	if(s == NULL){return NULL;}
	while((cur < bufEnd) && *s){
		*cur++ = *s++;
	}
	return cur;
}

/* Write the string S into the buffer while escaping all characters and wrapping
 * everything in quotes */
static char *writeStringEscaped(char *buf, char *bufEnd, const char *s){
	char *cur = buf;
	if((cur+1) >= bufEnd){return buf;}
	*cur++ = '\"';
	while(cur < bufEnd){
		const u8 c = *s++;
		switch(c){
		case 0:
			goto bufWriteStringExit;
		case '\a': // Bell
			*cur++ = '\\'; *cur++ = 'a';
			break;
		case '\b': // Backspace
			*cur++ = '\\'; *cur++ = 'b';
			break;
		case '\t': // Horiz. Tab
			*cur++ = '\\'; *cur++ = 't';
			break;
		case '\n': // Line Feed
			*cur++ = '\\'; *cur++ = 'n';
			break;
		case '\v': // Vert. Tab
			*cur++ = '\\'; *cur++ = 'v';
			break;
		case '\f': // Form Feed
			*cur++ = '\\'; *cur++ = 'f';
			break;
		case '\r': // Carriage Return
			*cur++ = '\\'; *cur++ = 'r';
			break;
		case 0x1B: // Escape
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

/* Write the integer V into BUF */
static char *writeInt(char *buf, char *bufEnd, i64 v){
	if(v < 0){
		if(buf < bufEnd){ *buf++ = '-'; }
		return writeInt(buf, bufEnd, -v);
	}else if(v >= 10){
		buf = writeInt(buf, bufEnd, v / 10);
	}
	if(buf < bufEnd){*buf++ = '0' + (v % 10);}
	return buf;
}

/* Write the unsigned integer V into BUF */
static char *writeUint(char *buf, char *bufEnd, u64 v){
	if(v >= 10){ buf = writeUint(buf, bufEnd, v / 10); }
	if(buf < bufEnd){ *buf++ = '0' + (v % 10);}
	return buf;
}

/* Write the integer V into BUF in hexadecimal notation */
static char *writeXint(char *buf, char *bufEnd, u64 v){
	if(v >= 16){ buf = writeXint(buf, bufEnd, v >> 4); }
	if(buf < bufEnd){
		const uint mv = v & 0xF;
		*buf++ = mv < 10 ? '0' + mv : ('A' - 10) + mv;
	}
	return buf;
}

/* Write out the floating point number V into BUF */
static char *writeFloat(char *buf, char *bufEnd, double v){
	double fract, integer;
	fract = fabs(modf(v, &integer));
	fract = round(fract * 100000.0) / 100000.0;
	if(fract > (1.0-DBL_EPSILON)){
		fract = 0;
		if(integer > 0){
			integer++;
		}else{
			integer--;
		}
	}
	char *cur = buf;
	if((v < 0) && (cur < bufEnd)){*cur++ = '-';}
	cur = writeInt(cur, bufEnd, (i64)fabs(integer));
	int zeroes = 0;
	int digits = 0;
	while((modf(fract, &integer) > DBL_EPSILON) && (++digits < 7)){
		fract *= 10.;
		if((i64)integer == 0){
 			zeroes++;
		}
	}
	zeroes = MAX(0, zeroes - 1);
	i64 ifract = round(fract);
	while(!(ifract % 10)){ifract /= 10; if(--digits < 0){break;}}
	if(cur < bufEnd){*cur++ = '.';}
	while((cur < bufEnd) && zeroes--){*cur++ = '0';}
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
				cur = writeTree(cur, bufEnd, va_arg(va, const lTree *));
				break;
			case 'M':
				cur = writeTreeDef(cur, bufEnd, va_arg(va, const lTree *));
				break;
			case 't': {
				const lSymbol *typeSym = getTypeSymbol(va_arg(va, const lVal *));
				if(typeSym){
					cur = writeString(cur, bufEnd, typeSym->c);
				}
				break; }
			case 'T': {
				const lVal *val = va_arg(va, const lVal *);
				cur = writeVal(cur, bufEnd, val, true);
				const lSymbol *typeSym = getTypeSymbol(val);
				if(typeSym){
					cur = writeString(cur, bufEnd, typeSym->c);
				}
				break; }
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
	char buf[8192];
	char *ret = vspf(buf,buf + sizeof(buf), format, va);
	fwrite(buf, ret - buf, 1, fp);
}

void fpf(FILE *fp, const char *format, ...){
	char buf[8192];
	va_list va;
	va_start(va ,format);
	char *ret = vspf(buf,buf + sizeof(buf), format, va);
	va_end(va);
	fwrite(buf, ret - buf, 1, fp);
}

void pf(const char *format, ...){
	char buf[8192];
	va_list va;
	va_start(va ,format);
	char *ret = vspf(buf,buf + sizeof(buf), format, va);
	va_end(va);
	fwrite(buf, ret - buf, 1, stdout);
}

void epf(const char *format, ...){
	char buf[8192];
	va_list va;
	va_start(va,format);
	char *ret = vspf(buf,buf + sizeof(buf), format, va);
	va_end(va);
	fwrite(buf, ret - buf, 1, stderr);
}
