#pragma once
#include "../nujel.h"

#define BYTECODE_STACK_SIZE 128
#define CLOSURE_STACK_SIZE 64


lVal *lBytecodeEval(lClosure *c, lVal *args, const lBytecodeArray *ops);
void lBytecodeArrayMark(const lBytecodeArray *v);