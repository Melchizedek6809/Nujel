/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <string.h>

typedef struct {
	u8 magic[4]; // NujI
	u32 imageSize;
	u8 version; // 1
	u8 padding[3];

	u8 data[];
} lImage;

typedef struct {
	i32 buffer;
	i32 length;
	i32 offset;
	u8 flags;
	u8 type;
} lImageBufferView;

typedef struct {
	i32 length;
	u8 flags;
	u8 data[];
} lImageBuffer;

typedef struct {
	u8 *start;
	i32 curOff;
	i32 size;
} writeImageContext;

static i32 dwordAlign(i32 eleSize){
	return (eleSize & 3) ? eleSize + 4 - (eleSize&3) : eleSize;
}

static lVal readVal(const lImage *img, i32 off, bool staticImage);

static const lSymbol *readSymbol(const lImage *img, i32 off, bool staticImage){
	(void)staticImage;
	return lSymS((const char *)&img->data[off]);
}

static lArray *readArray(const lImage *img, i32 off, bool staticImage){
	const i32 *in = (i32 *)((void *)&img->data[off]);
	const i32 len = in[0];
	lArray *out = lArrayAlloc(len);
	out->flags = in[1];
	in = &in[2];
	for(int i=0;i<len;i++){
		out->data[i] = readVal(img, in[i], staticImage);
	}
	return out;
}

static lBytecodeArray *readBytecodeArray(const lImage *img, i32 off, bool staticImage){
	const i32 *in = (i32 *)((void *)&img->data[off]);
	const i32 len = *in++;
	lBytecodeArray *ret = lBytecodeArrayAllocRaw();
	ret->literals = readArray(img, *in++, staticImage);
	if(staticImage){
		ret->data = (void *)in;
	} else {
		ret->data = malloc(len);
		memcpy(ret->data, in, len);
	}
	ret->dataEnd = &ret->data[len];
	return ret;
}

static lPair *readPair(const lImage *img, i32 off, bool staticImage){
	const i32 *pair = (const i32 *)((void *)&img->data[off]);
	lVal car = readVal(img,pair[0],staticImage);
	lVal cdr = readVal(img,pair[1],staticImage);
	lPair *ret = lPairAllocRaw();
	ret->car = car;
	ret->cdr = cdr;
	return ret;
}

static lBuffer *readBuffer(const lImage *img, i32 off, bool staticImage){
	lImageBuffer *imgBuf = (lImageBuffer *)((void *)&img->data[off]);
	bool immutable = imgBuf->flags & BUFFER_IMMUTABLE;
	lBuffer *buf = lBufferAlloc(imgBuf->length, immutable);
	if(staticImage && immutable){
		buf->buf = imgBuf->data;
		buf->flags |= BUFFER_STATIC;
	} else {
		buf->buf = malloc(imgBuf->length);
		memcpy(buf->buf, imgBuf->data, imgBuf->length);
	}
	return buf;
}

static lBufferView *readBufferView(const lImage *img, i32 off, bool staticImage){
	lImageBufferView *imgBuf = (lImageBufferView *)((void *)&img->data[off]);
	lBuffer *buf = readBuffer(img, imgBuf->buffer, staticImage);
	lBufferView *view = lBufferViewAllocRaw();
	view->buf = buf;
	view->offset = imgBuf->offset;
	view->length = imgBuf->length;
	view->flags = imgBuf->flags;
	view->type = imgBuf->type;
	return view;
}

static lTree *readTree(const lImage *img, i32 off, bool staticImage){
	const i32 *in = (i32 *)((void *)&img->data[off]);
	const i32 len = *in++;
	lTree *ret = lTreeAllocRaw();
	for(int i=0;i<len;i++){
		const lSymbol *s = readSymbol(img, *in++, staticImage);
		lVal v = readVal(img, *in++, staticImage);
		ret = lTreeInsert(ret, s, v);
	}
	return ret;
}

static lNFunc *readNFunc(const lImage *img, i32 off, bool staticImage){
	const lSymbol *sym = readSymbol(img, off, staticImage);
	for(uint i=0;i<lNFuncMax;i++){
		lNFunc *t = &lNFuncList[i];
		if(t == NULL){break;}
		if(t->name == sym){
			return t;
		}
	}
	return NULL;
}

static lClass *readType(const lImage *img, i32 off, bool staticImage){
	const lSymbol *sym = readSymbol(img, off, staticImage);
	for(uint i=0;i<countof(lClassList);i++){
		lClass *t = &lClassList[i];
		if(t == NULL){break;}
		if(t->name == sym){
			return t;
		}
	}
	return NULL;
}

static lVal readVal(const lImage *img, i32 off, bool staticImage){
	lVal rootValue = *((lVal *)((void *)&img->data[off]));
	switch(rootValue.type){
	case ltLambda:
	case ltMacro:
	case ltEnvironment:
		return lValException(lSymReadError, "Can't have rootValues of that Type (for now)", NIL);
	case ltFileHandle:
	case ltComment:
	case ltException:
		return lValException(lSymReadError, "Can't have rootValues of that Type", NIL);
	case ltNativeFunc:
		rootValue.vNFunc = readNFunc(img, rootValue.vInt, staticImage);
		return rootValue;
	case ltType:
		rootValue.vType = readType(img, rootValue.vInt, staticImage);
		return rootValue;
	case ltBytecodeArr:
		rootValue.vBytecodeArr = readBytecodeArray(img, rootValue.vInt, staticImage);
		return rootValue;
	case ltTree:
		return lValTree(readTree(img, rootValue.vInt, staticImage));
	case ltArray:
		rootValue.vArray = readArray(img, rootValue.vInt, staticImage);
		return rootValue;
	case ltPair:
		rootValue.vList = readPair(img, rootValue.vInt, staticImage);
		return rootValue;
	case ltBufferView:
		rootValue.vBufferView = readBufferView(img, rootValue.vInt, staticImage);
		return rootValue;
	case ltString:
	case ltBuffer:
		rootValue.vBuffer = readBuffer(img, rootValue.vInt, staticImage);
		return rootValue;
	case ltSymbol:
	case ltKeyword:
		rootValue.vSymbol = readSymbol(img, rootValue.vInt, staticImage);
		return rootValue;
	default:
		return rootValue;
	}
}

static lVal readImage(const lImage *img, bool staticImage){
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

	return readVal(img,0,staticImage);
}

static void ctxRealloc(writeImageContext *ctx, i32 eleSize){
	if((ctx->size - ctx->curOff) <= eleSize){
		ctx->size += eleSize;
		if(ctx->size & 0xFF){
			ctx->size += 0x100 - (ctx->size & 0xFF);
		}
		ctx->start = realloc(ctx->start, ctx->size);
	}
}

static i32 ctxAddVal(writeImageContext *ctx, lVal v);

static i32 ctxAddSymbol(writeImageContext *ctx, const lSymbol *v){
	i32 strLen = strnlen(v->c, sizeof(v->c));
	i32 eleSize = dwordAlign(strLen+1);
	ctxRealloc(ctx, eleSize);


	i32 curOff = ctx->curOff;
	ctx->curOff += eleSize;
	memcpy(&ctx->start[curOff], v->c, eleSize);
	ctx->start[ctx->curOff + strLen] = 0;
	return curOff;
}

static i32 ctxAddBuffer(writeImageContext *ctx, lBuffer *v){
	i32 eleSize = dwordAlign(sizeof(lImageBuffer) + v->length);
	ctxRealloc(ctx, eleSize);

	i32 curOff = ctx->curOff;
	lImageBuffer *out = (lImageBuffer *)((void *)&ctx->start[ctx->curOff]);
	ctx->curOff += eleSize;

	out->flags = v->flags;
	out->length = v->length;
	memcpy(out->data, v->data, v->length);
	return curOff;
}

static i32 ctxAddBufferView(writeImageContext *ctx, lBufferView *v){
	i32 eleSize = dwordAlign(sizeof(lImageBufferView));
	ctxRealloc(ctx, eleSize);

	i32 curOff = ctx->curOff;
	ctx->curOff += eleSize;
	const i32 buf = ctxAddBuffer(ctx, v->buf);
	lImageBufferView *out = (lImageBufferView *)((void *)&ctx->start[curOff]);
	out->buffer = buf;
	out->length = v->length;
	out->offset = v->offset;
	out->flags = v->flags;
	out->type = v->type;

	return curOff;
}

static i32 ctxAddPair(writeImageContext *ctx, lPair *v){
	i32 eleSize = dwordAlign(8);
	ctxRealloc(ctx, eleSize);

	i32 curOff = ctx->curOff;
	ctx->curOff += eleSize;

	const i32 car = ctxAddVal(ctx, v->car);
	const i32 cdr = ctxAddVal(ctx, v->cdr);
	i32 *out = (i32 *)((void *)&ctx->start[curOff]);
	out[0] = car;
	out[1] = cdr;
	return curOff;
}

static i32 ctxAddTreeVal(writeImageContext *ctx, i32 curOff, lTree *v){
	if(v == NULL){return curOff;}
	const i32 sym = ctxAddSymbol(ctx, v->key);
	const i32 val = ctxAddVal(ctx, v->value);
	i32 *out = (i32 *)((void *)&ctx->start[curOff]);
	out[0] = sym;
	out[1] = val;
	curOff += 8;
	curOff = ctxAddTreeVal(ctx, curOff, v->left);
	curOff = ctxAddTreeVal(ctx, curOff, v->right);
	return curOff;
}

static i32 ctxAddTree(writeImageContext *ctx, lTree *v){
	const int len = lTreeSize(v);
	const i32 eleSize = 4 + (4*2*len);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	i32 *out = (i32 *)((void *)&ctx->start[ctx->curOff]);
	*out = len;
	ctx->curOff += eleSize;
	ctxAddTreeVal(ctx, curOff + 4, v);
	return curOff;
}

static i32 ctxAddArray(writeImageContext *ctx, lArray *v){
	const i32 eleSize = 8 + (4 * v->length);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	i32 *out = (i32 *)((void *)&ctx->start[ctx->curOff]);
	ctx->curOff += eleSize;

	out[0] = v->length;
	out[1] = v->flags;
	for(int i=0;i<v->length;i++){
		const i32 off = ctxAddVal(ctx, v->data[i]);
		out = (i32 *)((void *)&ctx->start[curOff + 8 + i*4]);
		*out = off;
	}
	return curOff;
}

static i32 ctxAddBytecodeArray(writeImageContext *ctx, lBytecodeArray *v){
	int len = v->dataEnd - v->data;
	i32 eleSize = dwordAlign(8 + len);
	ctxRealloc(ctx, eleSize);

	i32 curOff = ctx->curOff;

	ctx->curOff += eleSize;

	*((i32 *)((void *)&ctx->start[curOff])) = len;
	const i32 arr = ctxAddArray(ctx, v->literals);
	*((i32 *)((void *)&ctx->start[curOff+4])) = arr;
	memcpy(&ctx->start[curOff+8], v->data, len);
	return curOff;
}

static i32 ctxAddVal(writeImageContext *ctx, lVal v){
	i32 eleSize = dwordAlign(sizeof(lVal));
	ctxRealloc(ctx, eleSize);

	i32 curOff = ctx->curOff;
	lVal *out = (lVal *)((void *)&ctx->start[ctx->curOff]);
	ctx->curOff += eleSize;

	switch(v.type){
	case ltLambda:
	case ltMacro:
	case ltEnvironment:
		exit(234);
	case ltFileHandle:
	case ltComment:
	case ltException:
		exit(234);
	case ltNativeFunc: {
		const u64 off = ctxAddSymbol(ctx, v.vNFunc->name);
		out = (lVal *)((void *)&ctx->start[curOff]);
		*out = v;
		out->vInt = off;
		break; }
	case ltType: {
		const u64 off = ctxAddSymbol(ctx, v.vType->name);
		out = (lVal *)((void *)&ctx->start[curOff]);
		*out = v;
		out->vInt = off;
		break; }
	case ltBytecodeArr: {
		const u64 off = ctxAddBytecodeArray(ctx, v.vBytecodeArr);
		out = (lVal *)((void *)&ctx->start[curOff]);
		*out = v;
		out->vInt = off;
		break; }
	case ltTree: {
		const u64 off = ctxAddTree(ctx, v.vTree->root);
		out = (lVal *)((void *)&ctx->start[curOff]);
		*out = v;
		out->vInt = off;
		break; }
	case ltArray: {
		const u64 off = ctxAddArray(ctx, v.vArray);
		out = (lVal *)((void *)&ctx->start[curOff]);
		*out = v;
		out->vInt = off;
		break; }
	case ltPair: {
		const u64 off = ctxAddPair(ctx, v.vList);
		out = (lVal *)((void *)&ctx->start[curOff]);
		*out = v;
		out->vInt = off;
		break; }
	case ltSymbol:
	case ltKeyword: {
		const u64 off = ctxAddSymbol(ctx, v.vSymbol);
		out = (lVal *)((void *)&ctx->start[curOff]);
		*out = v;
		out->vInt = off;
		break; }
	case ltBufferView: {
		const u64 off = ctxAddBufferView(ctx, v.vBufferView);
		out = (lVal *)((void *)&ctx->start[curOff]);
		*out = v;
		out->vInt = off;
		break; }
	case ltString:
	case ltBuffer: {
		const u64 off = ctxAddBuffer(ctx, v.vBuffer);
		out = (lVal *)((void *)&ctx->start[curOff]);
		*out = v;
		out->vInt = off;
		break; }
	default:
		*out = v;
		break;
	}
	return curOff;
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

	ctxAddVal(&ctx, rootValue);

	buf = realloc(buf, sizeof(lImage) + ctx.curOff);
	memcpy(buf->data, ctx.start, ctx.curOff);
	size += ctx.curOff;
	buf->imageSize = size;
	free(ctx.start);
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
	return readImage(val.vBuffer->buf, false);
}

void lOperationsImage(lClosure *c){
	lAddNativeFuncV(c,"image/serialize",   "(val)", "Serializes val into a binary representation that can be stored", lnfSerialize, 0);
	lAddNativeFuncV(c,"image/deserialize", "(buf)", "Deserializes buf into a value", lnfDeserialize, 0);
}
