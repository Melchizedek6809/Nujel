#ifndef NUJEL_COMMON
#define NUJEL_COMMON
/* Contains most typedefs and struct definitions
 * as well as a couple of macros for usage throughout
 * the codebase
 */

#ifdef __WATCOMC__
#define NORETURN
#else
#define NORETURN __attribute__((noreturn))
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define countof(x) (sizeof(x)/sizeof(*x))

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
		struct { float x,y,z; };
		struct { float v[3]; };
		struct { float yaw,pitch,roll; };
	};
};
typedef struct vec vec;

typedef uint8_t lBytecodeOp;

struct lBytecodeArray{
	lBytecodeOp *data;
	union {
		lBytecodeOp *dataEnd;
		struct lBytecodeArray *nextFree;
	};
};


typedef enum lType {
	ltNoAlloc = 0,
	ltComment,

	ltSymbol,
	ltKeyword,
	ltBool,
	ltInt,
	ltFloat,
	ltVec,

	ltPair,
	ltString,
	ltArray,
	ltTree,

	ltLambda,
	ltObject,
	ltMacro,
	ltThread,
	ltNativeFunc,
	ltBytecodeOp,
	ltBytecodeArr,

	ltGUIWidget
} lType;

typedef struct lArray   lArray;
typedef struct lClosure lClosure;
typedef struct lThread  lThread;
typedef struct lNFunc   lNFunc;
typedef struct lSymbol  lSymbol;
typedef struct lString  lString;
typedef struct lTree    lTree;
typedef struct lVec     lVec;
typedef struct lVal     lVal;
typedef struct lBytecodeArray lBytecodeArray;

typedef struct {
	lVal *car,*cdr;
} lPair;

struct lVal {
	u8 type;
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
	};
};

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

typedef enum closureType {
	closureDefault = 0,
	closureObject = 1,
	closureCall = 2,
	closureLet = 3,
	closureTry = 4
} closureType;

struct lClosure {
	union {
		lClosure *parent;
		lClosure *nextFree;
	};
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
	u16 rsp;
	u8 type;
};

struct lThread {
	lVal **valueStack;
	lClosure **closureStack;
	int valueStackSize;
	int sp;
	int csp;
	int closureStackSize;
	lBytecodeArray *text;
};

struct lString{
	const char *buf,*bufEnd;
	union {
		const char *data;
		lString *nextFree;
	};
	u8 flags;
};
#define HEAP_ALLOCATED 2

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
	lVal *doc;
	lVal *args;
	lSymbol *name;
};

struct lSymbol {
	union {
		char c[32];
		struct lSymbol *nextFree;
	};
};

#endif
