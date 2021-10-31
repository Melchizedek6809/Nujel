#pragma once
#include "../nujel.h"

lString *lStringNew    (const char *str, uint len);
lString *lStringDup    (      lString *s);
int      lStringLength (const lString *s);
lVal    *lValString    (const char *s);
