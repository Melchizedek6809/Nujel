#ifndef NUJEL_LIB_VM_BYTECODE
#define NUJEL_LIB_VM_BYTECODE

#include "../common.h"

typedef enum lOpcode {
	lopNOP             =  0x0,
	lopRet             =  0x1,
	lopIntByte         =  0x2,
	lopIntAdd          =  0x3,
	lopApply           =  0x4,
	lopGet             =  0x5,
	lopPushValExt      =  0x6,
	lopDef             =  0x7,
	lopSet             =  0x8,
	lopJmp             =  0x9,
	lopJt              =  0xA,
	lopJf              =  0xB,
	lopDup             =  0xC,
	lopDrop            =  0xD,
	lopUNUSEDX0E       =  0xE,
	lopUNUSEDX0F       =  0xF,
	lopUNUSEDX10       = 0x10,
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
	lopRootsSave       = 0x1B,
	lopRootsRestore    = 0x1C,
	lopUNUSED          = 0x1D,
	lopLessPred        = 0x1E,
	lopLessEqPred      = 0x1F,
	lopEqualPred       = 0x20,
	lopGreaterEqPred   = 0x21,
	lopGreaterPred     = 0x22,
	lopUNUSED23        = 0x23,
	lopPushNil         = 0x24,
	lopAdd             = 0x25,
	lopSub             = 0x26,
	lopMul             = 0x27,
	lopDiv             = 0x28,
	lopRem             = 0x29,
	lopZeroPred        = 0x2A
} lOpcode;

const char *lBytecodeGetOpcodeName(const lBytecodeOp op);
lBytecodeOp *lBytecodeReadOPSym(lBytecodeOp *ip, lSymbol **ret);
int lBytecodeGetOffset16(const lBytecodeOp *ip);

#endif
