#ifndef NUJEL_LIB_PRINTER
#define NUJEL_LIB_PRINTER
#include "common.h"
#include <stdio.h>
#include <stdarg.h>

char *vspf(char *buf, char *bufEnd, const char *format, va_list va);
char *spf(char *buf, char *bufEnd, const char *format, ...);
void vfpf(FILE *fp, const char *format, va_list va);
void fpf(FILE *f, const char *format, ...);
void epf(const char *format, ...);
void pf(const char *format, ...);

#endif