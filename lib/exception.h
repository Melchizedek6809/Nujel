#pragma once
#include "nujel.h"
#include <setjmp.h>

extern jmp_buf exceptionTarget;
extern lVal *exceptionValue;
extern int exceptionTargetDepth;

void lExceptionThrowRaw(lVal *v) __attribute__((noreturn));
void lExceptionThrow(const char *symbol, const char *error) __attribute__((noreturn));
void lExceptionThrowVal(const char *symbol, const char *error, lVal *v) __attribute__((noreturn));

void *lExceptionTry(void *(*body)(void *,void *), void *a, void *b);
