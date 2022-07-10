/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "bytecode.h"
#include "../common.h"
#include "../allocation/allocator.h"
#include "../allocation/symbol.h"

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

/* Read a symbol referenced at IP and store it in RET, retuns the new IP */
lBytecodeOp *lBytecodeReadOPSym(lBytecodeOp *ip, lSymbol **ret){
	int i = *ip++;
	i = (i << 8) | *ip++;
	i = (i << 8) | *ip++;
	*ret = lIndexSym(i);
	return ip;
}

/* Read an encoded signed 16-bit offset at ip */
int lBytecodeGetOffset16(const lBytecodeOp *ip){
	const int x = (ip[0] << 8) | ip[1];
	return (x < (1 << 15)) ? x : -((1<<16) - x);
}
