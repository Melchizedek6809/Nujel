#pragma once
#include "../nujel.h"

lClosure *lClosureNew       (lClosure *parent);

lVal     *lSearchClosureSym (lClosure *c, lVal *ret, const char *str, uint len);
lVal     *lDefineAliased    (lClosure *c, lVal *lNF, const char *sym);

lVal     *lSearchClosureSym (lClosure *c, lVal *v, const char *str, uint len);
lVal     *lGetClosureSym    (lClosure *c, const lSymbol *s);
bool      lHasClosureSym    (lClosure *c, const lSymbol *s, lVal **v);
void      lDefineClosureSym (lClosure *c, const lSymbol *s, lVal *v);
void      lSetClosureSym    (lClosure *c, const lSymbol *s, lVal *v);
void      lDefineVal        (lClosure *c, const char *str,  lVal *v);
