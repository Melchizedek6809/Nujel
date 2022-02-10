#ifndef NUJEL_LIB_TYPE_SYMBOL
#define NUJEL_LIB_TYPE_SYMBOL
#include "../nujel.h"

#define lGetSymbol(v) (((v == NULL) || (v->type != ltSymbol)) ? symNull : v->vSymbol)

lVal     *lValSymS     (const lSymbol *s);
lVal     *lValSym      (const char *s);

bool      lSymKeyword  (const lSymbol *s);

lVal     *lSymbolSearch(const char *s, uint len);

#endif
