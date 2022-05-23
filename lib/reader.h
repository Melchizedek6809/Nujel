#ifndef NUJEL_LIB_SEXPR_READER
#define NUJEL_LIB_SEXPR_READER
#include "nujel.h"
#include "type/string.h"

extern lClosure *readClosure;

lVal *lRead      (const char *str);
lVal *lReadValue (lString *s);
lVal *lReadList  (lString *s, bool rootForm);

#endif
