#ifndef NUJEL_LIB_VM_BYTECODE
#define NUJEL_LIB_VM_BYTECODE

#include "../common.h"

typedef enum lOpcode {
	lopNOP             =  0x0,
	lopRet             =  0x1,
	lopIntByte         =  0x2,
	lopIntAdd          =  0x3,
	lopApply           =  0x4,
	lopPushLVal        =  0x5,
	lopUNUSED6         =  0x6,
	lopUNUSED8         =  0x7,
	lopUNUSED1         =  0x8,
	lopJmp             =  0x9,
	lopJt              =  0xA,
	lopJf              =  0xB,
	lopDup             =  0xC,
	lopDrop            =  0xD,
	lopDef             =  0xE,
	lopSet             =  0xF,
	lopGet             = 0x10,
	lopCar             = 0x11,
	lopCdr             = 0x12,
	lopClosurePush     = 0x13,
	lopCons            = 0x14,
	lopLet             = 0x15,
	lopClosurePop      = 0x16,
	lopFnDynamic       = 0x17,
	lopMacroDynamic    = 0x18,
	lopTry             = 0x19,
	lopPushVal         = 0x1A,
	lopUNUSED1B        = 0x1B,
	lopUNUSED1C        = 0x1C,
	lopUNUSED          = 0x1D,
	lopLessPred        = 0x1E,
	lopLessEqPred      = 0x1F,
	lopEqualPred       = 0x20,
	lopGreaterEqPred   = 0x21,
	lopGreaterPred     = 0x22,
	lopPushSymbol      = 0x23,
	lopPushNil         = 0x24,
	lopAdd             = 0x25,
	lopSub             = 0x26,
	lopMul             = 0x27,
	lopDiv             = 0x28,
	lopRem             = 0x29,
	lopZeroPred        = 0x2A
} lOpcode;

const char *lBytecodeGetOpcodeName(const lBytecodeOp op);
lBytecodeOp *lBytecodeReadOPVal(lBytecodeOp *ip, lVal **ret);
lBytecodeOp *lBytecodeReadOPSym(lBytecodeOp *ip, lSymbol **ret);
int lBytecodeGetOffset16(const lBytecodeOp *ip);

#endif
