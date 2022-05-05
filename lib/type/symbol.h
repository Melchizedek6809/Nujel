#ifndef NUJEL_LIB_TYPE_SYMBOL
#define NUJEL_LIB_TYPE_SYMBOL
#include "../nujel.h"

extern lSymbol *symNull;
static inline const lSymbol *lGetSymbol(const lVal *v){
	return ((v == NULL) || (v->type != ltSymbol))
		? symNull
		: v->vSymbol;
}

lVal     *lValSymS      (const lSymbol *s);
lVal     *lValSym       (const char    *s);

lVal     *lValKeywordS  (const lSymbol *s);
lVal     *lValKeyword   (const char    *s);

#endif
