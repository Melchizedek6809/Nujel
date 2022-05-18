#ifndef NUJEL_LIB_VM_BYTECODE
#define NUJEL_LIB_VM_BYTECODE

typedef enum lOpcode {
	lopNOP             =  0x0,
	lopRet             =  0x1,
	lopIntByte         =  0x2,
	lopIntAdd          =  0x3,
	lopApplyNew        =  0x4,
	lopPushLVal        =  0x5,
	lopUNUSED6         =  0x6,
	lopUNUSED7         =  0x7,
	lopApply           =  0x8,
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
	lopUNUSED4         = 0x17,
	lopUNUSED5         = 0x18,
	lopTry             = 0x19,
	lopApplyDynamic    = 0x1A,
	lopRootsSave       = 0x1B,
	lopRootsRestore    = 0x1C,
	lopUNUSED          = 0x1D,
	lopLessPred        = 0x1E,
	lopLessEqPred      = 0x1F,
	lopEqualPred       = 0x20,
	lopGreaterEqPred   = 0x21,
	lopGreaterPred     = 0x22,
	lopPushSymbol      = 0x23,
	lopPushNil         = 0x24,
	lopFn              = 0x25,
	lopMacroAst        = 0x26
	/* BE SURE TO ADD A CASE TO lBytecodeOpLength!!! */
} lOpcode;

#endif