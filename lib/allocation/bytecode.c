/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "bytecode.h"
#include "garbage-collection.h"
#include "symbol.h"
#include "../printer.h"
#include "../vm/bytecode.h"

#include <stdlib.h>

/* Return the overall length of opcode op */
static int lBytecodeOpLength(lBytecodeOp op){
	switch(op){
	default:
		epf("Unknown bytecodeOp length: %x\n",op);
		exit(3);
	case lopNOP:
	case lopCar:
	case lopCdr:
	case lopCons:
	case lopRet:
	case lopIntAdd:
	case lopDup:
	case lopDrop:
	case lopClosurePush:
	case lopLet:
	case lopClosurePop:
	case lopRootsSave:
	case lopRootsRestore:
	case lopLessPred:
	case lopLessEqPred:
	case lopEqualPred:
	case lopGreaterEqPred:
	case lopGreaterPred:
	case lopPushNil:
		return 1;
	case lopApplyDynamic:
	case lopApplyDynamicNew:
	case lopIntByte:
		return 2;
	case lopTry:
	case lopJmp:
	case lopJf:
	case lopJt:
		return 3;
	case lopPushSymbol:
	case lopDef:
	case lopSet:
	case lopGet:
	case lopPushLVal:
		return 4;
	case lopApply:
	case lopApplyNew:
		return 5;
	case lopMacroAst:
	case lopFn:
		return 4*3+1;
	}
}

/* Mark all objects references within v, should only be called from the GC */
void lBytecodeArrayMarkRefs(const lBytecodeArray *v){
	for(const lBytecodeOp *c = v->data; c < v->dataEnd; c += lBytecodeOpLength(*c)){
		switch(*c){
		default: break;
		case lopPushSymbol:
		case lopDef:
		case lopGet:
		case lopSet:
			if(&c[3] >= v->dataEnd){break;}
			lSymbolGCMark(lIndexSym((c[1] << 16) | (c[2] << 8) | c[3]));
			break;
		case lopApply:
		case lopApplyNew:
			if(&c[4] >= v->dataEnd){break;}
			lValGCMark(lIndexVal((c[ 2] << 16) | (c[ 3] << 8) | c[ 4]));
			break;
		case lopMacroAst:
		case lopFn:
			if(&c[12] >= v->dataEnd){break;}
			lValGCMark(lIndexVal((c[ 1] << 16) | (c[ 2] << 8) | c[ 3]));
			lValGCMark(lIndexVal((c[ 4] << 16) | (c[ 5] << 8) | c[ 6]));
			lValGCMark(lIndexVal((c[ 7] << 16) | (c[ 8] << 8) | c[ 9]));
			lValGCMark(lIndexVal((c[10] << 16) | (c[11] << 8) | c[12]));
			break;
		case lopPushLVal:
			if(&c[3] >= v->dataEnd){break;}
			lValGCMark(lIndexVal((c[1] << 16) | (c[2] << 8) | c[3]));
			break;
		}
	}
}
