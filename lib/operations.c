/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "operations.h"

/* These shouldn't be exported since we should only really call into
 | lOperationsBase and determine which operations need addition in there.
 */
void lOperationsArithmetic (lClosure *c);
void lOperationsArray      (lClosure *c);
void lOperationsBytecode   (lClosure *c);
void lOperationsCore       (lClosure *c);
void lOperationsMath       (lClosure *c);
void lOperationsSpecial    (lClosure *c);
void lOperationsString     (lClosure *c);
void lOperationsTree       (lClosure *c);
void lOperationsVector     (lClosure *c);

void lOperationsBase(lClosure *c){
	lOperationsArithmetic(c);
	lOperationsMath(c);
	lOperationsArray(c);
	lOperationsBytecode(c);
	lOperationsCore(c);
	lOperationsSpecial(c);
	lOperationsString(c);
	lOperationsTree(c);
	lOperationsVector(c);
}
