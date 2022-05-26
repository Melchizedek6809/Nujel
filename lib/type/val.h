#ifndef NUJEL_LIB_TYPE_VAL
#define NUJEL_LIB_TYPE_VAL
#include "../nujel.h"
#include "../type-system.h"

lVal     *lValSymS      (const lSymbol *s);
lVal     *lValKeywordS  (const lSymbol *s);
lVal     *lValSym       (const char    *s);
lVal     *lValKeyword   (const char    *s);

int       lValCompare      (const lVal *a, const lVal *b);
bool      lValEqual        (const lVal *a, const lVal *b);
i64       lValGreater      (const lVal *a, const lVal *b);
lVal     *lValBool         (bool v);
lVal     *lValInt          (i64 v);
lVal     *lValFloat        (double v);
lVal     *lValVec          (const vec v);
lVal     *lValTree         (lTree *v);
lVal     *lValObject       (lClosure *v);
lVal     *lValLambda       (lClosure *v);
lVal     *lValString       (const char *s);
lVal     *lValStringLen    (const char *s, int len);
lVal     *lValStringNoCopy (const char *s, int len);
lVal     *lValStringError  (const char *bufStart, const char *bufEnd, const char *errStart, const char *err, const char *errEnd);

static inline lVal *lValComment(){return lValAlloc(ltComment);}

extern lSymbol *symNull;
static inline const lSymbol *lGetSymbol(const lVal *v){
	return ((v == NULL) || (v->type != ltSymbol))
		? symNull
		: v->vSymbol;
}

lString  *lStringNew       (const char *str, uint len);
lString  *lStringDup       (      lString *s);
int       lStringLength    (const lString *s);


#endif
