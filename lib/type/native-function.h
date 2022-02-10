#ifndef NUJEL_LIB_TYPE_NFUNC
#define NUJEL_LIB_TYPE_NFUNC
#include "../nujel.h"

lVal     *lAddNativeFunc      (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));
lVal     *lAddSpecialForm     (lClosure *c, const char *sym, const char *args, const char *doc, lVal *(*func)(lClosure *,lVal *));

#endif
