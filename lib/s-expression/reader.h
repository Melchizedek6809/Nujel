#pragma once
#include "../nujel.h"
#include "../collection/string.h"

lVal *lRead             (const char *str);
lVal *lReadString       (lString *s);
void  lOperationsReader (lClosure *c);
