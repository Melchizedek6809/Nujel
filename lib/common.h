#ifndef NUJEL_COMMON
#define NUJEL_COMMON
/* Contains most typedefs and struct definitions
 * as well as a couple of macros for usage throughout
 * the codebase
 */

#ifdef __WATCOMC__
#define NORETURN
#define likely(x)   x
#define unlikely(x) x
#else
#define NORETURN __attribute__((noreturn))
#define likely(x)      __builtin_expect(!!(x), 1)
#define unlikely(x)    __builtin_expect(!!(x), 0)
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define countof(x) (sizeof(x)/sizeof(*x))
#define typeswitch(v) switch(v ? v->type : ltNoAlloc)

typedef unsigned int     uint;

typedef uint64_t          u64;
typedef uint32_t          u32;
typedef uint16_t          u16;
typedef uint8_t            u8;

typedef  int64_t          i64;
typedef  int32_t          i32;
typedef  int16_t          i16;
typedef  int8_t            i8;

struct vec {
	union {
		struct { float x,y,z,w; };
		struct { float v[4]; };
		struct { float yaw,pitch,roll,_w; };
	};
};
typedef struct vec vec;


typedef enum {
	ltNoAlloc = 0,
	ltComment = 1,

	ltSymbol = 2,
	ltKeyword = 3,
	ltBool = 4,
	ltInt = 5,
	ltFloat = 6,
	ltVec = 7,

	ltPair = 8,
	ltString = 9,
	ltArray = 10,
	ltTree = 11,

	ltLambda = 12,
	ltObject = 13,
	ltMacro = 14,
	ltThread = 15,
	ltNativeFunc = 16,
	ltBytecodeOp = 17,
	ltBytecodeArr = 18,

	ltBuffer = 19,
	ltBufferView = 20,

	ltGUIWidget
} lType;

typedef struct lBuffer lBuffer;
typedef struct lBufferView lBufferView;
typedef struct lArray   lArray;
typedef struct lClosure lClosure;
typedef struct lThread  lThread;
typedef struct lNFunc   lNFunc;
typedef struct lSymbol  lSymbol;
typedef struct lTree    lTree;
typedef struct lVec     lVec;
typedef struct lVal     lVal;
typedef struct lBytecodeArray lBytecodeArray;
typedef uint8_t lBytecodeOp;
typedef lBuffer lString;


typedef enum {
	lbvtUndefined = 0,
	lbvtS8,
	lbvtU8,
	lbvtS16,
	lbvtU16,
	lbvtS32,
	lbvtU32,
	lbvtS64,
	lbvtF32,
	lbvtF64
} lBufferViewType;

struct lBufferView {
	union {
		lBuffer *buf;
		lBufferView *nextFree;
	};
	size_t offset;
	size_t length;
	lBufferViewType type;
	u8 flags;
};
#define BUFFER_VIEW_IMMUTABLE 1

struct lBytecodeArray{
	lBytecodeOp *data;
	lArray *literals;
	union {
		lBytecodeOp *dataEnd;
		struct lBytecodeArray *nextFree;
	};
	u8 flags;
};

struct lBuffer {
	union {
		void *buf;
		const char *data;
		lBuffer *nextFree;
	};
	i32 length;
	u8 flags;
};
#define BUFFER_IMMUTABLE 1

struct lArray {
	lVal **data;
	union {
		lArray *nextFree;
		struct {
			i32 length;
			u8 flags;
		};
	};
};
#define ARRAY_IMMUTABLE 1


struct lSymbol {
	union {
		char c[32];
		struct lSymbol *nextFree;
	};
};

typedef struct {
	lVal *car,*cdr;
} lPair;

struct lVal {
	u32 type;
	union {
		bool            vBool;
		lPair           vList;
		i64             vInt;
		double          vFloat;
		vec             vVec;
		lBytecodeOp     vBytecodeOp;
		lBytecodeArray *vBytecodeArr;
		lArray         *vArray;
		lTree          *vTree;
		lString        *vString;
		const lSymbol  *vSymbol;
		lClosure       *vClosure;
		lNFunc         *vNFunc;
		void           *vPointer;
		lVal           *nextFree;
		lBuffer        *vBuffer;
		lBufferView    *vBufferView;
	};
};

typedef enum closureType {
	closureDefault = 0,
	closureObject = 1,
	closureCall = 2,
	closureLet = 3,
	closureTry = 4,
	closureRoot = 5,
} closureType;

struct lClosure {
	lClosure *parent;
	lClosure *nextFree;
	lTree *data, *meta;
	lBytecodeArray *text;
	lBytecodeOp *ip;
	union {
		lVal *args;
		lVal *exceptionHandler;
	};
	const lSymbol *name;
	lClosure *caller;
	int sp;
	u8 type;
};

struct lThread {
	lVal **valueStack;
	lClosure **closureStack;
	lBytecodeArray *text;
	int valueStackSize;
	int closureStackSize;
	int sp;
	int csp;
};

struct lTree {
	lTree *left;
	lTree *right;
	union {
		const lSymbol *key;
		lTree *nextFree;
	};
	lVal *value;
	i16 height;
	u8 flags;
};
#define TREE_IMMUTABLE 1

struct lNFunc {
	lVal *(*fp)(lClosure *, lVal *);
	lTree *meta;
	lVal *args;
	lSymbol *name;
};

#endif
