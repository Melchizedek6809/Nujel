#pragma once
#include "nujel.h"
#include <setjmp.h>

extern jmp_buf exceptionTarget;
extern lVal *exceptionValue;

void lExceptionThrowRaw(lVal *v);
void lExceptionThrow(const char *symbol, const char *error);
void lExceptionThrowVal(const char *symbol, const char *error, lVal *v);
void lExceptionInit();
