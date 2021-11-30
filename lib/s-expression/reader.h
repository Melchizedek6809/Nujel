#pragma once
#include "../nujel.h"
#include "../collection/string.h"

extern lClosure *readClosure;

lVal *lRead             (const char *str);
lVal *lReadValue        (lString *s);
lVal *lReadList         (lString *s, bool rootForm);
void  lOperationsReader (lClosure *c);
