#pragma once
#include "../nujel.h"

#define VALUE_STACK_SIZE 1024
#define CALL_STACK_SIZE   128

lVal *lBytecodeEval(lClosure *c, lVal *args, const lBytecodeArray *ops);
void lBytecodeArrayMark(const lBytecodeArray *v);
