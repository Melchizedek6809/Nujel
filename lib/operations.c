/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

void lOperationsBase(lClosure *c){
	lOperationsArithmetic(c);
	lOperationsBuffer(c);
	lOperationsArray(c);
	lOperationsBytecode(c);
	lOperationsCore(c);
	lOperationsSpecial(c);
	lOperationsString(c);
	lOperationsTree(c);
}
