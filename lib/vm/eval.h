#ifndef NUJEL_LIB_VM_EVAL
#define NUJEL_LIB_VM_EVAL
#include "../nujel.h"

lVal *lBytecodeEval(lClosure *c, lVal *args, lBytecodeArray *ops, bool trace);

#endif
