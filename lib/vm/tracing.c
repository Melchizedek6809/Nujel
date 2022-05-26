/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "tracing.h"

#include "bytecode.h"
#include "eval.h"
#include "../printer.h"

#include <stdlib.h>
#include <string.h>

const char *getIndent(int d){
	static char *buf = NULL;
	static char *bufEnd = NULL;
	if(buf == NULL){
		buf = malloc(128);
		bufEnd = &buf[127];
		if(buf == NULL){exit(1);}
		memset(buf, ' ', 128);
		*bufEnd=0;
	}
	return &bufEnd[-MIN(126, d * 4)];
}

void lBytecodeTrace(const lThread *ctx, lBytecodeOp *ip, const lBytecodeArray *ops){
	if(*ip == lopApply){
		lVal *fun = NULL;
		lBytecodeReadOPVal(ip+2, &fun);
		pf("%s[$%s %V]\n", getIndent(ctx->csp), lBytecodeGetOpcodeName(*ip), fun);
	} else if(*ip == lopPushLVal){
		lVal *val = NULL;
		lBytecodeReadOPVal(ip+1, &val);
		pf("%s[$%s %V]\n", getIndent(ctx->csp), lBytecodeGetOpcodeName(*ip), val);
	} else if(*ip == lopGet){
		lSymbol *s = NULL;
		lBytecodeReadOPSym(ip+1, &s);
		pf("%s[$%s '%s]\n", getIndent(ctx->csp), lBytecodeGetOpcodeName(*ip), s->c);
	} else {
		pf("%s[$%s]\n", getIndent(ctx->csp), lBytecodeGetOpcodeName(*ip));
	}
	pf("%s [IP:%i CSP:%i SP:%i] [Text: %p]\n", getIndent(ctx->csp), (ip - ops->data), (i64)ctx->csp, (i64)ctx->sp, ops->data);
	pf("%s [Data: %m]\n", getIndent(ctx->csp), ctx->closureStack[ctx->csp]->data);
	for(int i=ctx->csp;i>=0;i--){
		//pf("Clo[%u] = %i\n", i, (i64)(ctx->closureStack[i] ? ctx->closureStack[i]->type : 0));
	}
	pf("%s [", getIndent(ctx->csp));
	for(int i=ctx->sp-1;i>=0;i--){
		pf("%V%s", ctx->valueStack[i], i>0?" ":"");
	}
	pf("]\n");
	if(ctx->csp < 0){
		epf("CSP Error!");
		exit(1);
	}
	if(ctx->sp < 0){
		epf("SP Error!");
		exit(1);
	}
}
