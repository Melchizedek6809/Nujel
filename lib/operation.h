#ifndef NUJEL_LIB_OPERATION
#define NUJEL_LIB_OPERATION

#include "nujel.h"


lVal *lnfCat     (lClosure *c, lVal *v);
lVal *lnfTry     (lClosure *c, lVal *v);
lVal *lnfDo      (lClosure *c, lVal *v);
lVal *lnfArrRef  (lClosure *c, lVal *v);
lVal *lnfArrNew  (lClosure *c, lVal *v);
lVal *lnfTreeGet (lClosure *c, lVal *v);
lVal *lnfTreeNew (lClosure *c, lVal *v);

u64 getMSecs     ();

void lOperationsAllocation (lClosure *c);
void lOperationsArithmetic (lClosure *c);
void lOperationsArray      (lClosure *c);
void lOperationsBinary     (lClosure *c);
void lOperationsBytecode   (lClosure *c);
void lOperationsClosure    (lClosure *c);
void lOperationsEval       (lClosure *c);
void lOperationsList       (lClosure *c);
void lOperationsMath       (lClosure *c);
void lOperationsReader     (lClosure *c);
void lOperationsPredicate  (lClosure *c);
void lOperationsSpecial    (lClosure *c);
void lOperationsString     (lClosure *c);
void lOperationsTime       (lClosure *c);
void lOperationsTree       (lClosure *c);
void lOperationsVector     (lClosure *c);

#endif
