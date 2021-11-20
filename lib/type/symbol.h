#pragma once
#include "../nujel.h"

#define lGetSymbol(v) (((v == NULL) || (v->type != ltSymbol)) ? symNull : v->vSymbol)

lVal     *lValSymS     (const lSymbol *s);
lVal     *lValSym      (const char *s);

int       lSymCmp      (const lVal *a,const lVal *b);
int       lSymEq       (const lSymbol *a,const lSymbol *b);

bool      lSymVariadic (const lSymbol *s);
bool      lSymKeyword  (const lSymbol *s);

lVal     *lSymbolSearch(const char *s, uint len);
