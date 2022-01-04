#pragma once
#include "../nujel.h"

#define BYTECODE_STACK_SIZE 128

lVal *lBytecodeEval(lClosure *c, lVal *args, const lBytecodeArray *ops);
