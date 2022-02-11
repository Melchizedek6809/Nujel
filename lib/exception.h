#ifndef NUJEL_LIB_EXCEPTION
#define NUJEL_LIB_EXCEPTION

#include "nujel.h"
#include <setjmp.h>

extern jmp_buf exceptionTarget;
extern lVal *exceptionValue;
extern int exceptionTargetDepth;

void  lExceptionThrowRaw    (lVal *v) __attribute__((noreturn));
void  lExceptionThrow       (const char *symbol, const char *error) __attribute__((noreturn));
void  lExceptionThrowVal    (const char *symbol, const char *error, lVal *v) __attribute__((noreturn));
void  lExceptionThrowValClo (const char *symbol, const char *error, lVal *v, lClosure *c) __attribute__((noreturn));
void *lExceptionTryExit     (void *(*body)(void *,void *), void *a, void *b);
void *lExceptionTryCatch    (void *(*body)(void *,void *), void *a, void *b, void (*handler)(lVal *exceptionValue));

#endif
