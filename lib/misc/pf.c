/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "pf.h"

#include "../allocation/closure.h"
#include "../allocation/native-function.h"
#include "../allocation/symbol.h"
#include "../collection/list.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

static char *writeVal(char *buf, char *bufEnd, const lVal *v, bool display);

static char *writeTreeRec(char *cur, char *bufEnd, const lTree *v){
	if((v == NULL) || (v->key == NULL)){return cur;}

	cur = writeTreeRec(cur, bufEnd, v->left);
	cur = spf(cur,bufEnd,"%s %v ",v->key->c, v->value);
	cur = writeTreeRec(cur, bufEnd, v->right);

	return cur;
}

static char *writeTree(char *cur, char *bufEnd, const lTree *v){
	cur = spf(cur,bufEnd,"@[");
	char *new = writeTreeRec(cur, bufEnd, v);
	if(new != cur){
		cur = new;
		cur[-1] = ']';
	}else if(cur < bufEnd){
		*cur++ = ']';
	}
	return cur;
}

static char *writeTreeDef(char *cur, char *bufEnd, const lTree *v){
	if(v == NULL){return cur;}

	cur = writeTreeDef(cur, bufEnd, v->left);
	cur = spf(cur,bufEnd,"[def %s %v]\n",v->key->c, v->value);
	return writeTreeDef(cur, bufEnd, v->right);
}

static char *writeArray(char *cur, char *bufEnd, const lArray *v){
	cur = spf(cur, bufEnd, "#[");
	if(v && v->data != NULL){
		for(int i=0;i<v->length;i++){
			cur = spf(cur, bufEnd, "%v%s", v->data[i], (i < (v->length-1)) ? " " : "");
		}
	}
	return spf(cur, bufEnd, "]");
}

static char *writeBytecodeArray(char *cur, char *bufEnd, const lBytecodeArray *v){
	cur = spf(cur, bufEnd, "#[");
	if(v && v->data != NULL){
		for(const lBytecodeOp *c = v->data; c < v->dataEnd; c++){
			cur = spf(cur, bufEnd, "#$%x%s", (i64)(*c & 0xFF), (c < (v->dataEnd - 1)) ? " " : "");
		}
	}
	return spf(cur, bufEnd, "]");
}

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

static char *writeVal(char *buf, char *bufEnd, const lVal *v, bool display){
	char *cur = buf;

	if(v == NULL){
		return spf(buf,bufEnd,"#nil");
	}
	switch(v->type){
	default:
		return buf;
	case ltNoAlloc:
		return spf(cur, bufEnd,"#zzz");
	case ltBool:
		return spf(cur, bufEnd,"%s", v->vBool ? "#t" : "#f");
	case ltObject:
		return spf(cur, bufEnd, "[ω %T]", v->vClosure->data);
	case ltMacro:
	case ltLambda:
		if(v->vClosure && v->vClosure->name){
			return spf(cur, bufEnd, "%s", v->vClosure->name->c);
		}else{
			return spf(cur, bufEnd, "#%s_%u", v->type == ltLambda ? "λ" : "μ", lClosureID(v->vClosure));
		}
	case ltPair:
		return writePair(cur, bufEnd, v);
	case ltTree:
		return writeTree(cur, bufEnd, v->vTree);
	case ltArray:
		return writeArray(cur, bufEnd, v->vArray);
	case ltBytecodeArr:
		return writeBytecodeArray(cur, bufEnd, &v->vBytecodeArr);
	case ltBytecodeOp:
		return spf(cur , bufEnd, "#$%x" , (i64)(v->vBytecodeOp & 0xFF));
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

static char *writeString(char *buf, char *bufEnd, const char *s){
	char *cur = buf;
	if(s == NULL){return NULL;}
	while((cur < bufEnd) && *s){
		*cur++ = *s++;
	}
	return cur;
}

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
		case '\e': // Escape
			*cur++ = '\\'; *cur++ = 'e';
			break;
		case '"':
			*cur++ = '\\'; *cur++ = '"';
			break;
		case '\'':
			*cur++ = '\\'; *cur++ = '\'';
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

static char *writeUint(char *buf, char *bufEnd, u64 v){
	if(v >= 10){ buf = writeUint(buf, bufEnd, v / 10); }
	if(buf < bufEnd){ *buf++ = '0' + (v % 10);}
	return buf;
}

static char *writeXint(char *buf, char *bufEnd, u64 v){
	if(v >= 16){ buf = writeXint(buf, bufEnd, v >> 4); }
	if(buf < bufEnd){
		const uint mv = v & 0xF;
		*buf++ = mv < 10 ? '0' + mv : ('A' - 10) + mv;
	}
	return buf;
}

static char *writeFloat(char *buf, char *bufEnd, double v){
	double fract, integer;
	fract = fabs(modf(v, &integer));
	char *cur = buf;
	if((v < 0) && (cur < bufEnd)){*cur++ = '-';}
	cur = writeInt(cur, bufEnd, (i64)fabs(integer));
	int zeroes = 0;
	int digits = 0;
	fract = round(fract * 100000.0) / 100000.0;
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
			case 't':
				cur = writeTree(cur, bufEnd, va_arg(va, const lTree *));
				break;
			case 'T':
				cur = writeTreeDef(cur, bufEnd, va_arg(va, const lTree *));
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
