/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "operation.h"

void lOperationsArithmetic (lClosure *c);
void lOperationsArray      (lClosure *c);
void lOperationsBinary     (lClosure *c);
void lOperationsBytecode   (lClosure *c);
void lOperationsCasting    (lClosure *c);
void lOperationsCore       (lClosure *c);
void lOperationsMath       (lClosure *c);
void lOperationsSpecial    (lClosure *c);
void lOperationsString     (lClosure *c);
void lOperationsTree       (lClosure *c);
void lOperationsVector     (lClosure *c);

/* Add all the core native functions to c, without IO or stdlib */
void lAddCoreFuncs(lClosure *c){
	lOperationsArithmetic(c);
	lOperationsMath(c);
	lOperationsArray(c);
	lOperationsBinary(c);
	lOperationsBytecode(c);
	lOperationsCasting(c);
	lOperationsCore(c);
	lOperationsSpecial(c);
	lOperationsString(c);
	lOperationsTree(c);
	lOperationsVector(c);
}
