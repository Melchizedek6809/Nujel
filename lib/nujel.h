#ifndef NUJEL_LIB_NUJEL
#define NUJEL_LIB_NUJEL
#include "common.h"
#include "allocation/allocator.h"
#include "allocation/garbage-collection.h"
#include "allocation/roots.h"

#include <setjmp.h>

extern bool    lVerbose;
extern jmp_buf exceptionTarget;
extern lVal   *exceptionValue;
extern int     exceptionTargetDepth;

void  lExceptionThrowRaw    (lVal *v) NORETURN;
void  lExceptionThrowValClo (const char *symbol, const char *error, lVal *v, lClosure *c) NORETURN;

void      lInit    ();
lClosure *lNewRoot ();
lVal     *lLambda  (lClosure *c, lVal *args, lVal *lambda);
lVal     *lApply   (lClosure *c, lVal *args, lVal *fun);
lClosure *lLoad    (lClosure *c, const char *expr);

lVal *lnfCat     (lClosure *c, lVal *v);
lVal *lnfArrNew  (lClosure *c, lVal *v);
lVal *lnfTreeNew (lClosure *c, lVal *v);
lVal *lnfVec     (lClosure *c, lVal *v);

int       lListLength (lVal *v);
lVal *lCons(lVal *car, lVal *cdr);

static inline lVal *lCar(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.car : NULL;
}

static inline lVal *lCdr(lVal *v){
	return (v != NULL) && (v->type == ltPair) ? v->vList.cdr : NULL;
}

static inline lVal *lCaar  (lVal *v){return lCar(lCar(v));}
static inline lVal *lCadr  (lVal *v){return lCar(lCdr(v));}
static inline lVal *lCdar  (lVal *v){return lCdr(lCar(v));}
static inline lVal *lCddr  (lVal *v){return lCdr(lCdr(v));}
static inline lVal *lCaadr (lVal *v){return lCar(lCar(lCdr(v)));}
static inline lVal *lCdadr (lVal *v){return lCdr(lCar(lCdr(v)));}
static inline lVal *lCadar (lVal *v){return lCar(lCdr(lCar(v)));}
static inline lVal *lCaddr (lVal *v){return lCar(lCdr(lCdr(v)));}
static inline lVal *lCdddr (lVal *v){return lCdr(lCdr(lCdr(v)));}
static inline lVal *lCadddr(lVal *v){return lCar(lCdr(lCdr(lCdr(v))));}


#endif
