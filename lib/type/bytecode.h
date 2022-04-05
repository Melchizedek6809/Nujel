#ifndef NUJEL_LIB_TYPE_BYTECODE
#define NUJEL_LIB_TYPE_BYTECODE
#include "../nujel.h"

#define VALUE_STACK_SIZE  512
#define CALL_STACK_SIZE   128
#define ROOT_STACK_SIZE    32

typedef enum lOpcode {
	lopNOP             =  0x0,
	lopRet             =  0x1,
	lopIntByte         =  0x2,
	lopIntAdd          =  0x3,
	lopDebugPrintStack =  0x4,
	lopPushLVal        =  0x5,
	lopMakeList        =  0x6,
	lopEval            =  0x7,
	lopApply           =  0x8,
	lopJmp             =  0x9,
	lopJt              =  0xA,
	lopJf              =  0xB,
	lopDup             =  0xC,
	lopDrop            =  0xD,
	lopDef             =  0xE,
	lopSet             =  0xF,
	lopGet             = 0x10,
	lopLambda          = 0x11,
	lopMacro           = 0x12,
	lopClosurePush     = 0x13,
	lopClosureEnter    = 0x14,
	lopLet             = 0x15,
	lopClosurePop      = 0x16,
	lopCall            = 0x17,
	lopTry             = 0x18,
	lopThrow           = 0x19,
	lopApplyDynamic    = 0x1A,
	lopRootsPush       = 0x1B,
	lopRootsPop        = 0x1C,
	lopRootsPeek       = 0x1D,
	lopLessPred        = 0x1E,
	lopLessEqPred      = 0x1F,
	lopEqualPred       = 0x20,
	lopGreaterEqPred   = 0x21,
	lopGreaterPred     = 0x22,
	lopPushSymbol      = 0x23,
	lopPushNil         = 0x24
} lOpcode;

lVal *lBytecodeEval(lClosure *c, lVal *args, const lBytecodeArray *ops);
void lBytecodeArrayMark(const lBytecodeArray *v);
void lBytecodeLink(lClosure *c);

#endif
