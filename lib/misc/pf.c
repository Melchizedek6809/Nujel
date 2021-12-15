/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include "pf.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

static char *writeString(char *buf, char *bufEnd, const char *s){
	char *cur = buf;
	while((buf < bufEnd) && *s){ *cur++ = *s++; }
	return cur;
}

char *writeStringEscaped(char *buf, char *bufEnd, const char *s){
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

#include <stdio.h>
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
	//printf("%f = %i . [%i] %lli\n",v, (int)integer, zeroes, ifract);
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
			}
			format++;
		}else{
			*cur++ = *format++;
		}
	}
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

void pf(const char *format, ...){
	char buf[8192];
	va_list va;
	va_start(va ,format);
	char *ret = vspf(buf,buf + sizeof(buf), format, va);
	va_end(va);
	fwrite(buf, ret - buf, 1, stdout);
}
