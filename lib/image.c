/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

typedef enum {
	lITNil = 0,
	lITSymbol,
	lITArray,
	lITClosure,
	lITTree,
	lITTreeRoot,
	lITBytecodeArray,
	lITBuffer,
	lITBufferView,
	lITPair
} lImageTableTypes;

typedef struct {
	uint8_t T;
	uint32_t imageOffset;
	uint32_t elementCount;
	uint32_t tableSize; // Useful because of variably sized elements
} lImageTable;

typedef struct {
	uint8_t magic[4]; // NujI
	uint8_t version; // 1

	uint32_t rootClosure;
	uint32_t tableCount;
	uint32_t imageSize;
	lImageTable tables[];
} lImage;
