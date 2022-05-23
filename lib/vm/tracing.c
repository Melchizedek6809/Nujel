/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "tracing.h"

#include "bytecode.h"
#include "../printer.h"

#include <stdlib.h>

void lBytecodeTrace(const lThread *ctx, const lBytecodeOp *ip, const lBytecodeArray *ops){
	pf("[OP: %u]\n [IP: %x, CSP:%i SP:%i]\n", (ip - ops->data), (i64)*ip, (i64)ctx->csp, (i64)ctx->sp);
	for(int i=ctx->csp;i>=0;i--){
		pf("C %u: %i\n", i, (i64)(ctx->closureStack[i] ? ctx->closureStack[i]->type : 0));
	}
	for(int i=ctx->sp-1;i>=0;i--){
		pf("V %u: %V\n", i, ctx->valueStack[i]);
	}
	pf("\n");
	if(ctx->csp < 0){
		epf("CSP Error!");
		exit(1);
	}
	if(ctx->sp < 0){
		epf("SP Error!");
		exit(1);
	}
}
