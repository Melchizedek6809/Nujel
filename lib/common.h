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

#define PI    (3.1415926535897932384626433832795)
#define PI180 (3.1415926535897932384626433832795 / 180.0)
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))
#define MINMAX(a, b, v) (MAX(a,MIN(b,v)))
#define countof(x) (sizeof(x)/sizeof(*x))

typedef unsigned int     uint;
typedef unsigned short ushort;
typedef unsigned char   uchar;

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

typedef struct lBytecodeArray{
	lBytecodeOp *data,*dataEnd;
} lBytecodeArray;


typedef enum lType {
	ltNoAlloc = 0,
	ltComment,
	ltValueStack,
	ltCallStack,

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
	ltNativeFunc,
	ltSpecialForm,
	ltBytecodeOp,
	ltBytecodeArr,

	ltGUIWidget
} lType;

typedef struct lArray   lArray;
typedef struct lClosure lClosure;
typedef struct lNFunc   lNFunc;
typedef struct lSymbol  lSymbol;
typedef struct lString  lString;
typedef struct lTree    lTree;
typedef struct lVec     lVec;
typedef struct lVal     lVal;

typedef struct {
	lVal *car,*cdr;
} lPair;

struct lVal {
	u8 type;
	union {
		bool           vBool;
		lPair          vList;
		i64            vInt;
		double         vFloat;
		vec            vVec;
		lBytecodeOp    vBytecodeOp;
		lBytecodeArray vBytecodeArr;
		lArray        *vArray;
		lTree         *vTree;
		lString       *vString;
		const lSymbol *vSymbol;
		lClosure      *vClosure;
		lNFunc        *vNFunc;
		void          *vPointer;

		lVal          *nextFree;
	};
};

struct lArray {
	union {
		lVal **data;
		lArray *nextFree;
	};
	i32 length;
};

typedef enum closureType {
	closureDefault = 0,
	closureObject = 1,
	closureConstant = 2,
	closureCall = 3,
	closureLet = 4,
	closureTry = 5,
	closureRet = 6
} closureType;

struct lClosure {
	union {
		lClosure *parent;
		lClosure *nextFree;
	};
	lTree *data;
	union {
		lVal *text;
		const lBytecodeOp *ip;
	};
	union {
		lVal *args;
		int sp;
	};
	lVal *doc;
	lClosure *caller;
	const lSymbol *name;
	u8 type;
};

struct lString{
	const char *buf,*bufEnd;
	union {
		const char *data;
		lString *nextFree;
	};
	u16 flags;
};
#define HEAP_ALLOCATED 1

struct lTree {
	lTree *left;
	lTree *right;
	const lSymbol *key;
	int height;
	union {
		lVal *value;
		lTree *nextFree;
	};
};

struct lNFunc {
	union {
		lVal *(*fp)(lClosure *, lVal *);
		lNFunc *nextFree;
	};
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
