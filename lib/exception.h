#ifndef NUJEL_LIB_EXCEPTION
#define NUJEL_LIB_EXCEPTION

#include "nujel.h"
#include <setjmp.h>

extern jmp_buf exceptionTarget;
extern lVal *exceptionValue;
extern int exceptionTargetDepth;

extern bool breakQueued;

void  lExceptionThrowRaw    (lVal *v) NORETURN;
void  lExceptionThrow       (const char *symbol, const char *error) NORETURN;
void  lExceptionThrowValClo (const char *symbol, const char *error, lVal *v, lClosure *c) NORETURN;
void *lExceptionTryExit     (void *(*body)(void *,void *), void *a, void *b);
void *lExceptionTryCatch    (void *(*body)(void *,void *), void *a, void *b, void (*handler)(lVal *exceptionValue));

static inline void lCheckBreak(){
        if(!breakQueued){return;}
	breakQueued = false;
	lExceptionThrow(":break","A break has been triggered");
}

#endif
