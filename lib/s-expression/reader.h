#pragma once
#include "../nujel.h"
#include "../collection/string.h"

lVal *lRead             (const char *str);
lVal *lReadValue        (lString *s);
lVal *lReadList         (lString *s);
void  lOperationsReader (lClosure *c);
