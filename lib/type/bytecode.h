#ifndef NUJEL_LIB_TYPE_BYTECODE
#define NUJEL_LIB_TYPE_BYTECODE
#include "../nujel.h"

#define VALUE_STACK_SIZE  512
#define CALL_STACK_SIZE   128
#define ROOT_STACK_SIZE    32

lVal *lBytecodeEval(lClosure *c, lVal *args, const lBytecodeArray *ops);
void lBytecodeArrayMark(const lBytecodeArray *v);

#endif
