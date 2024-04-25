/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#ifndef NUJEL_LIB_NUJEL_IMAGE
#define NUJEL_LIB_NUJEL_IMAGE

#include "nujel.h"
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

typedef struct {
	u8 magic[4]; // NujI
	u8 data[];
} lImage;

typedef enum {
	litNil = 0,

	litSymbol,
	litKeyword,

	litInt8,
	litInt16,
	litInt32,
	litInt64,

	litFloat,

	litPair,
	litArray,
	litTree,

	litLambda,
	litMacro,
	litNativeFunc,
	litEnvironment,

	litString,
	litBuffer,
	litBufferView,
	litBytecodeArr,

	litFileHandle,
	litType,

	litTrue,
	litFalse,

	litMap

} lImageType;

typedef struct {
	i32 buffer;
	i32 length;
	i32 offset;
	u8 flags;
	u8 type;
} lImageBufferView;

typedef struct {
	i32 parent;
	i32 data;
	i32 meta;
	i32 text;
	i32 args;
	i32 ip;
	u16 sp;
	u8 type;
} lImageClosure;

#endif
