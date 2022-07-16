/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "../nujel-private.h"
#endif

#include <stdlib.h>
#include <string.h>

const char *lBytecodeGetOpcodeName(const lBytecodeOp op){
	switch(op){
	case lopNOP:             return "nop";
	case lopRet:             return "ret";
	case lopIntByte:         return "push/int/byte";
	case lopIntAdd:          return "add/int";
	case lopPushVal:         return "push/val";
	case lopPushValExt:      return "push/val/ext";
	case lopJmp:             return "jmp";
	case lopJt:              return "jt";
	case lopJf:              return "jf";
	case lopDup:             return "dup";
	case lopDrop:            return "drop";
	case lopDef:             return "def";
	case lopSet:             return "set";
	case lopGet:             return "get";
	case lopCar:             return "car";
	case lopCdr:             return "cdr";
	case lopClosurePush:     return "closure/push";
	case lopCons:            return "cons";
	case lopLet:             return "let";
	case lopClosurePop:      return "closure/pop";
	case lopTry:             return "try";
	case lopApply:           return "apply/dynamic";
	case lopLessPred:        return "<";
	case lopLessEqPred:      return "<=";
	case lopEqualPred:       return "==";
	case lopGreaterPred:     return ">";
	case lopGreaterEqPred:   return ">=";
	case lopPushNil:         return "push/nil";
	case lopFnDynamic:       return "fn";
	case lopMacroDynamic:    return "macro";
	case lopAdd:             return "+";
	case lopSub:             return "-";
	case lopMul:             return "*";
	case lopDiv:             return "/";
	case lopRem:             return "rem";
	default:                 return ":UNKNOWN-OP";
	}
}

const char *getIndent(int d){
	static char *buf = NULL;
	static char *bufEnd = NULL;
	if(buf == NULL){
		buf = malloc(128);
		if (unlikely(buf == NULL)) {
			return "";
		}
		bufEnd = &buf[127];
		if(buf == NULL){exit(1);}
		memset(buf, ' ', 128);
		*bufEnd=0;
	}
	return &bufEnd[-MIN(126, d * 4)];
}

void lBytecodeTrace(const lThread *ctx, lBytecodeOp *ip, const lBytecodeArray *ops){
	pf("%s[$%s]\n", getIndent(ctx->csp), lBytecodeGetOpcodeName(*ip));
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
