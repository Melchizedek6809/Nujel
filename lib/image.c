/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <string.h>

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
	uint8_t flags;
	uint32_t offset;
	uint32_t size;
} lImageTableBuffer;

typedef struct {
	uint8_t T;
	uint32_t imageOffset;
	uint32_t elementCount;
	uint32_t tableSize; // Useful because of variably sized elements
} lImageTable;

typedef struct {
	uint8_t magic[4]; // NujI
	uint8_t version; // 1
	uint32_t tableCount;
	uint32_t imageSize;

	lVal rootValue;
	lImageTable tables[];
} lImage;

static lVal readImage(const lImage *img){
	if(unlikely(img == NULL)){
		return lValExceptionSimple();
	}
	if(unlikely((img->magic[0] != 'N')
	   || (img->magic[1] != 'u')
	   || (img->magic[2] != 'j')
	   || (img->magic[3] != 'I'))){
		return lValException(lSymReadError, "Invalid Magic prefix in Nujel Image", NIL);
	}
	if(unlikely(img->version != 1)){
		return lValException(lSymReadError, "Unsupported Version in Nujel Image", lValInt(img->version));
	}
	const size_t size = img->imageSize;
	if(unlikely(img->imageSize < sizeof(lImage))){
		return lValException(lSymReadError, "Invalid Image size", lValInt(size));
	}

	if(unlikely(img->tableCount > 0)){
		return lValException(lSymReadError, "Can't read image Tables (for now)", NIL);
	}
	lVal rootValue = img->rootValue;
	switch(rootValue.type){
	case ltSymbol:
	case ltKeyword:
	case ltPair:
	case ltArray:
	case ltTree:
	case ltLambda:
	case ltMacro:
	case ltEnvironment:
	case ltString:
	case ltBuffer:
	case ltBufferView:
	case ltBytecodeArr:
		return lValException(lSymReadError, "Can't have rootValues of that Type", NIL);
	case ltFileHandle:
	case ltComment:
	case ltNativeFunc:
	case ltException:
	case ltType:
		return lValException(lSymReadError, "Can't have rootValues of that Type (for now)", NIL);
	default:
		return rootValue;
	}
}

typedef struct {
	lBuffer **buffer;
	uint32_t bufferCount;
	uint32_t bufferSize;
} writeImageContext;

static uint32_t ctxAddBuffer(writeImageContext *ctx, lBuffer *v){
	if(ctx->bufferSize <= (1+ctx->bufferCount)){
		ctx->bufferSize += 64;
		ctx->buffer = realloc(ctx->buffer, ctx->bufferSize * sizeof(void *));
	}
	ctx->buffer[ctx->bufferCount] = v;
	return ctx->bufferCount++;
}

static lImage *ctxWriteBufferTable(writeImageContext *ctx, lImage *img){
	if(ctx->bufferCount == 0){
		return img;
	}
	return img;
}

static lImage *ctxWriteTables(writeImageContext *ctx, lImage *img){
	img = ctxWriteBufferTable(ctx, img);
	return img;
}

static lImage *writeImage(lVal rootValue){
	size_t size = sizeof(lImage);
	writeImageContext ctx;
	memset(&ctx, 0, sizeof(ctx));
	lImage *buf = malloc(size);

	buf->magic[0] = 'N';
	buf->magic[1] = 'u';
	buf->magic[2] = 'j';
	buf->magic[3] = 'I';
	buf->version = 1;

	buf->tableCount = 0;
	buf->imageSize = size;
	switch(rootValue.type){
	case ltSymbol:
	case ltKeyword:
	case ltPair:
	case ltArray:
	case ltTree:
	case ltLambda:
	case ltMacro:
	case ltEnvironment:
	case ltBufferView:
	case ltBytecodeArr:
		free(buf);
		return NULL;
	case ltFileHandle:
	case ltComment:
	case ltNativeFunc:
	case ltException:
	case ltType:
		free(buf);
		return NULL;
	case ltString:
	case ltBuffer:
		buf->rootValue = rootValue;
		buf->rootValue.vBuffer = (lBuffer *)((uint64_t)ctxAddBuffer(&ctx, rootValue.vBuffer));
		break;
	default:
		buf->rootValue = rootValue;
		break;
	}
	buf = ctxWriteTables(&ctx, buf);
	free(ctx.buffer);
	return buf;
}

static lVal lnfSerialize(lVal val){
	lImage *img = writeImage(val);
	if(unlikely(img == NULL)){
		return lValException(lSymTypeError, "Can't serialize that", NIL);
	} else {
		lBuffer *buf = lBufferAlloc(img->imageSize, true);
		buf->buf = img;
		return lValAlloc(ltBuffer, buf);
	}
}

static lVal lnfDeserialize(lVal val){
	reqBuffer(val);
	return readImage(val.vBuffer->buf);
}

void lOperationsImage(lClosure *c){
	lAddNativeFuncV(c,"image/serialize",   "(val)", "Serializes val into a binary representation that can be stored", lnfSerialize, 0);
	lAddNativeFuncV(c,"image/deserialize", "(buf)", "Deserializes buf into a value", lnfDeserialize, 0);
}
