#ifndef NUJEL_LIB_EXCEPTION
#define NUJEL_LIB_EXCEPTION

#include "nujel.h"
#include <setjmp.h>

extern jmp_buf exceptionTarget;
extern lVal *exceptionValue;
extern int exceptionTargetDepth;

void  lExceptionThrowRaw    (lVal *v) NORETURN;
void  lExceptionThrowValClo (const char *symbol, const char *error, lVal *v, lClosure *c) NORETURN;

#endif
