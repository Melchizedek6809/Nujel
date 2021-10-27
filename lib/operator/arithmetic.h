#pragma once
#include "../nujel.h"

lVal *lnfInfix (lClosure *c, lVal *v);
void lAddInfix(lVal *v);

void lOperationsArithmetic(lClosure *c);
