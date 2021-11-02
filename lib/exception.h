#pragma once
#include "nujel.h"
#include <setjmp.h>

extern jmp_buf exceptionTarget;
extern lVal *exceptionValue;

void lExceptionThrow(const char *symbol, const char *error);
void lExceptionThrowRaw(lVal *v);
void lExceptionInit();
