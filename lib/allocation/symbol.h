#ifndef NUJEL_LIB_ALLOC_SYMBOL
#define NUJEL_LIB_ALLOC_SYMBOL
#include "../nujel.h"

#define SYM_MAX (1<<14)
extern lSymbol  lSymbolList [SYM_MAX];
extern lSymbol *lSymbolFFree;
extern uint     lSymbolActive;
extern uint     lSymbolMax;

extern lSymbol *symType;
extern lSymbol *symArguments;
extern lSymbol *symCode;
extern lSymbol *symData;

extern lSymbol *symNull;
extern lSymbol *symQuote;
extern lSymbol *symQuasiquote;
extern lSymbol *symUnquote;
extern lSymbol *symUnquoteSplicing;
extern lSymbol *symArr;
extern lSymbol *symTreeNew;
extern lSymbol *symDocumentation;

void      lSymbolInit   ();
void      lSymbolFree   (lSymbol *s);
lSymbol  *lSymS         (const char *s);
lSymbol  *lSymSM        (const char *s);
lSymbol  *lSymSL        (const char *s, uint len);
lSymbol  *getTypeSymbol (const lVal *a);
lSymbol  *getTypeSymbolT(const lType T);

static inline int lSymIndex(const lSymbol *s){
	return s - lSymbolList;
}
static inline lSymbol *lIndexSym(uint i){
	return (i >= lSymbolMax) ? NULL : &lSymbolList[i];
}

#endif
