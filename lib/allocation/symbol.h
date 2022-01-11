#pragma once
#include "../nujel.h"

#define SYM_MAX (1<<14)
extern lSymbol  lSymbolList [SYM_MAX];
extern lSymbol *lSymbolFFree;
extern uint     lSymbolActive;
extern uint     lSymbolMax;

extern lSymbol *symNull,*symQuote,*symQuasiquote, *symUnquote, *symUnquoteSplicing, *symArr,*symIf,*symCond,*symDo,*symMinus,*symLambda,*symLambdAst,*symTreeNew;

void      lSymbolInit  ();
void      lSymbolFree  (lSymbol *s);
lSymbol  *lSymS        (const char *s);
lSymbol  *lSymSL       (const char *s, uint len);
lSymbol  *getTypeSymbol(const lVal *a);

static inline int lSymIndex(const lSymbol *s){return s - lSymbolList;}
static inline lSymbol *lIndexSym(uint i){return (i >= lSymbolMax) ? NULL : &lSymbolList[i];}
