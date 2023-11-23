/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include <string.h>

typedef struct {
	u8 magic[4]; // NujI
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
	i32 parent;
	i32 data;
	i32 meta;
	i32 text;
	i32 args;
	i32 ip;
	u16 sp;
	u8 type;
} lImageClosure;

typedef struct {
	i32 len;
	i32 size;
	i32 *key;
	void **val;
} readImageMap;

typedef struct {
	i32 len;
	i32 size;
	void **key;
	i32 *val;
} writeImageMap;

typedef struct {
	u8 *start;
	i32 curOff;
	i32 size;
	writeImageMap map;
} writeImageContext;

static lVal readVal(readImageMap *map, const lImage *img, i32 off, bool staticImage);
static i32 ctxAddVal(writeImageContext *ctx, lVal v);

static i32 writeMapGet(writeImageMap *map, void *key){
	for(int i=0;i<map->len;i++){
		if(map->key[i] == key){
			return map->val[i];
		}
	}
	return 0;
}

static void writeMapSet(writeImageMap *map, void *key, i32 val){
	if(map->len+1 >= map->size){
		map->size += 32;
		map->key = realloc(map->key, map->size * sizeof(void *));
		map->val = realloc(map->val, map->size * sizeof(i32));
	}
	map->key[map->len] = key;
	map->val[map->len] = val;
	map->len++;
}

static void *readMapGet(readImageMap *map, i32 key){
	for(int i=0;i<map->len;i++){
		if(map->key[i] == key){
			return map->val[i];
		}
	}
	return NULL;
}

static void readMapSet(readImageMap *map, i32 key, void *val){
	if(map->len+1 >= map->size){
		map->size += 32;
		map->key = realloc(map->key, map->size * sizeof(void *));
		map->val = realloc(map->val, map->size * sizeof(i32));
	}
	map->key[map->len] = key;
	map->val[map->len] = val;
	map->len++;
}

static i32 dwordAlign(i32 eleSize){
	return (eleSize & 3) ? eleSize + 4 - (eleSize&3) : eleSize;
}

static const lSymbol *readSymbol(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	(void)staticImage;
	(void)map;
	return lSymS((const char *)&img->data[off]);
}

static lArray *readArray(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lArray *)mapP; }
	const i32 *in = (i32 *)((void *)&img->data[off]);
	const i32 len = in[0];
	lArray *out = lArrayAlloc(len);
	out->flags = in[1];
	in = &in[2];
	for(int i=0;i<len;i++){
		out->data[i] = readVal(map, img, in[i], staticImage);
	}
	readMapSet(map, off, out);
	return out;
}

static lBytecodeArray *readBytecodeArray(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lBytecodeArray *)mapP; }
	const i32 *in = (i32 *)((void *)&img->data[off]);
	const i32 len = *in++;
	lBytecodeArray *ret = lBytecodeArrayAllocRaw();
	ret->literals = readArray(map, img, *in++, staticImage);
	if(staticImage){
		ret->data = (void *)in;
	} else {
		ret->data = malloc(len);
		memcpy(ret->data, in, len);
	}
	ret->dataEnd = &ret->data[len];
	readMapSet(map, off, ret);
	return ret;
}

static lPair *readPair(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lPair *)mapP; }
	const i32 *pair = (const i32 *)((void *)&img->data[off]);
	lVal car = readVal(map, img,pair[0],staticImage);
	lVal cdr = readVal(map, img,pair[1],staticImage);
	lPair *ret = lPairAllocRaw();
	ret->car = car;
	ret->cdr = cdr;
	readMapSet(map, off, ret);
	return ret;
}

static lBuffer *readBuffer(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lBuffer *)mapP; }
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
	readMapSet(map, off, buf);
	return buf;
}

static lBufferView *readBufferView(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lBufferView *)mapP; }
	lImageBufferView *imgBuf = (lImageBufferView *)((void *)&img->data[off]);
	lBuffer *buf = readBuffer(map, img, imgBuf->buffer, staticImage);
	lBufferView *view = lBufferViewAllocRaw();
	view->buf = buf;
	view->offset = imgBuf->offset;
	view->length = imgBuf->length;
	view->flags = imgBuf->flags;
	view->type = imgBuf->type;
	readMapSet(map, off, view);
	return view;
}

static lTree *readTree(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lTree *)mapP; }
	const i32 *in = (i32 *)((void *)&img->data[off]);
	const i32 len = *in++;
	lTree *ret = lTreeAllocRaw();
	for(int i=0;i<len;i++){
		const lSymbol *s = readSymbol(map, img, *in++, staticImage);
		lVal v = readVal(map, img, *in++, staticImage);
		ret = lTreeInsert(ret, s, v);
	}
	readMapSet(map, off, ret);
	return ret;
}

static lNFunc *readNFunc(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	(void)map;
	const lSymbol *sym = readSymbol(map, img, off, staticImage);
	for(uint i=0;i<lNFuncMax;i++){
		lNFunc *t = &lNFuncList[i];
		if(t == NULL){break;}
		if(t->name == sym){
			return t;
		}
	}
	return NULL;
}

static lClass *readType(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	(void)map;
	const lSymbol *sym = readSymbol(map, img, off, staticImage);
	for(uint i=0;i<countof(lClassList);i++){
		lClass *t = &lClassList[i];
		if(t == NULL){break;}
		if(t->name == sym){
			return t;
		}
	}
	return NULL;
}

static lClosure *readClosure(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lClosure *)mapP; }
	lImageClosure *clo = (lImageClosure *)((void *)&img->data[off]);
	lClosure *ret = lClosureAllocRaw();

	ret->args = clo->args ? readVal(map, img, clo->args, staticImage) : NIL;
	ret->data = clo->data ? readTree(map, img, clo->data, staticImage) : NULL;
	ret->meta = clo->meta ? readTree(map, img, clo->meta, staticImage) : NULL;
	ret->parent = clo->parent ? readClosure(map, img, clo->parent, staticImage) : NULL;
	ret->sp = clo->sp;
	ret->text = clo->text ? readBytecodeArray(map, img, clo->text, staticImage) : NULL;
	ret->type = clo->type;
	ret->ip = &ret->text->data[clo->ip];
	if(ret->text && (ret->ip >= ret->text->dataEnd)){
		ret->ip = NULL;
		return NULL;
	}

	readMapSet(map, off, ret);
	return ret;
}

static lVal readVal(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	lVal rootValue = *((lVal *)((void *)&img->data[off]));
	switch(rootValue.type){
	case ltComment:
	case ltException:
		return lValException(lSymReadError, "Can't have rootValues of that Type", NIL);
	case ltFileHandle:
		switch(rootValue.vInt){
		case 0:
			return lValFileHandle(stdin);
		case 1:
			return lValFileHandle(stdout);
		case 2:
			return lValFileHandle(stderr);
		default:
			return lValException(lSymReadError, "Can't serialize file handles other than stdin, stdout or stderr", NIL);
		}
	case ltNativeFunc:
		rootValue.vNFunc = readNFunc(map, img, rootValue.vInt, staticImage);
		return rootValue;
	case ltType:
		rootValue.vType = readType(map, img, rootValue.vInt, staticImage);
		return rootValue;
	case ltBytecodeArr:
		rootValue.vBytecodeArr = readBytecodeArray(map, img, rootValue.vInt, staticImage);
		return rootValue;
	case ltTree:
		return lValTree(readTree(map, img, rootValue.vInt, staticImage));
	case ltArray:
		rootValue.vArray = readArray(map, img, rootValue.vInt, staticImage);
		return rootValue;
	case ltPair:
		rootValue.vList = readPair(map, img, rootValue.vInt, staticImage);
		return rootValue;
	case ltBufferView:
		rootValue.vBufferView = readBufferView(map, img, rootValue.vInt, staticImage);
		return rootValue;
	case ltLambda:
	case ltMacro:
	case ltEnvironment:
		rootValue.vClosure = readClosure(map, img, rootValue.vInt, staticImage);
		return rootValue;
	case ltString:
	case ltBuffer:
		rootValue.vBuffer = readBuffer(map, img, rootValue.vInt, staticImage);
		return rootValue;
	case ltSymbol:
	case ltKeyword:
		rootValue.vSymbol = readSymbol(map, img, rootValue.vInt, staticImage);
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
	readImageMap map;
	memset(&map, 0, sizeof(map));

	lVal ret = readVal(&map, img,0,staticImage);
	free(map.key);
	free(map.val);
	return ret;
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

static i32 ctxAddSymbol(writeImageContext *ctx, const lSymbol *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 strLen = strnlen(v->c, sizeof(v->c));
	const i32 eleSize = dwordAlign(strLen+1);
	ctxRealloc(ctx, eleSize);


	const i32 curOff = ctx->curOff;
	ctx->curOff += eleSize;
	memcpy(&ctx->start[curOff], v->c, eleSize);
	ctx->start[ctx->curOff + strLen] = 0;
	writeMapSet(&ctx->map, (void *)v, curOff);
	return curOff;
}

static i32 ctxAddTreeVal(writeImageContext *ctx, i32 curOff, lTree *v){
	if(v == NULL){return curOff;}
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 sym = ctxAddSymbol(ctx, v->key);
	const i32 val = ctxAddVal(ctx, v->value);
	i32 *out = (i32 *)((void *)&ctx->start[curOff]);
	out[0] = sym;
	out[1] = val;
	curOff += 8;
	curOff = ctxAddTreeVal(ctx, curOff, v->left);
	curOff = ctxAddTreeVal(ctx, curOff, v->right);
	writeMapSet(&ctx->map, (void *)v, curOff);
	return curOff;
}

static i32 ctxAddTree(writeImageContext *ctx, lTree *v){
	if(v == NULL){return 0;}
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const int len = lTreeSize(v);
	const i32 eleSize = 4 + (4*2*len);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	i32 *out = (i32 *)((void *)&ctx->start[ctx->curOff]);
	*out = len;
	ctx->curOff += eleSize;
	ctxAddTreeVal(ctx, curOff + 4, v);
	writeMapSet(&ctx->map, (void *)v, curOff);
	return curOff;
}

static i32 ctxAddArray(writeImageContext *ctx, lArray *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

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
	writeMapSet(&ctx->map, (void *)v, curOff);
	return curOff;
}

static i32 ctxAddBytecodeArray(writeImageContext *ctx, lBytecodeArray *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	int len = v->dataEnd - v->data;
	i32 eleSize = dwordAlign(8 + len);
	ctxRealloc(ctx, eleSize);

	i32 curOff = ctx->curOff;

	ctx->curOff += eleSize;

	*((i32 *)((void *)&ctx->start[curOff])) = len;
	const i32 arr = ctxAddArray(ctx, v->literals);
	*((i32 *)((void *)&ctx->start[curOff+4])) = arr;
	memcpy(&ctx->start[curOff+8], v->data, len);
	writeMapSet(&ctx->map, (void *)v, curOff);
	return curOff;
}

static i32 ctxAddClosure(writeImageContext *ctx, lClosure *v){
	if(v == NULL){return 0;}
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = dwordAlign(sizeof(lImageClosure));
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	lImageClosure *out = (lImageClosure *)((void *)&ctx->start[ctx->curOff]);
	ctx->curOff += eleSize;

	out->args = ctxAddVal(ctx, v->args);
	out->ip   = (i32)((u8 *)v->ip - (u8 *)v->text->data);
	out->data = ctxAddTree(ctx, v->data);
	out->meta = ctxAddTree(ctx, v->meta);
	out->parent = ctxAddClosure(ctx, v->parent);
	out->sp   = v->sp;
	out->text = ctxAddBytecodeArray(ctx, v->text);
	out->type = v->type;

	writeMapSet(&ctx->map, (void *)v, curOff);
	return curOff;
}

static i32 ctxAddBuffer(writeImageContext *ctx, lBuffer *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = dwordAlign(sizeof(lImageBuffer) + v->length);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	lImageBuffer *out = (lImageBuffer *)((void *)&ctx->start[ctx->curOff]);
	ctx->curOff += eleSize;

	out->flags = v->flags;
	out->length = v->length;
	memcpy(out->data, v->data, v->length);
	writeMapSet(&ctx->map, (void *)v, curOff);
	return curOff;
}

static i32 ctxAddBufferView(writeImageContext *ctx, lBufferView *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = dwordAlign(sizeof(lImageBufferView));
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	ctx->curOff += eleSize;
	const i32 buf = ctxAddBuffer(ctx, v->buf);
	lImageBufferView *out = (lImageBufferView *)((void *)&ctx->start[curOff]);
	out->buffer = buf;
	out->length = v->length;
	out->offset = v->offset;
	out->flags = v->flags;
	out->type = v->type;

	writeMapSet(&ctx->map, (void *)v, curOff);
	return curOff;
}

static i32 ctxAddPair(writeImageContext *ctx, lPair *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = dwordAlign(8);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	ctx->curOff += eleSize;

	const i32 car = ctxAddVal(ctx, v->car);
	const i32 cdr = ctxAddVal(ctx, v->cdr);
	i32 *out = (i32 *)((void *)&ctx->start[curOff]);
	out[0] = car;
	out[1] = cdr;
	writeMapSet(&ctx->map, (void *)v, curOff);
	return curOff;
}

static i32 ctxAddVal(writeImageContext *ctx, lVal v){
	i32 eleSize = dwordAlign(sizeof(lVal));
	ctxRealloc(ctx, eleSize);

	i32 curOff = ctx->curOff;
	lVal *out = (lVal *)((void *)&ctx->start[ctx->curOff]);
	ctx->curOff += eleSize;

	switch(v.type){
	case ltComment:
	case ltException:
		exit(234);
	case ltFileHandle:
		*out = v;
		if(v.vFileHandle == stdin){
			out->vInt = 0;
		} else if(v.vFileHandle == stdout){
			out->vInt = 1;
		} else if(v.vFileHandle == stderr){
			out->vInt = 2;
		} else {
			out->vInt = 0xFF;
		}
		break;
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
	case ltLambda:
	case ltMacro:
	case ltEnvironment: {
		const u64 off = ctxAddClosure(ctx, v.vClosure);
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

static lImage *writeImage(lVal rootValue, i32 *outSize){
	size_t size = sizeof(lImage);
	writeImageContext ctx;
	memset(&ctx, 0, sizeof(ctx));
	lImage *buf = malloc(size);

	buf->magic[0] = 'N';
	buf->magic[1] = 'u';
	buf->magic[2] = 'j';
	buf->magic[3] = 'I';

	ctxAddVal(&ctx, rootValue);

	buf = realloc(buf, sizeof(lImage) + ctx.curOff);
	memcpy(buf->data, ctx.start, ctx.curOff);
	size += ctx.curOff;
	*outSize = size;
	free(ctx.start);
	free(ctx.map.key);
	free(ctx.map.val);
	return buf;
}

static lVal lnfSerialize(lVal val){
	i32 size;
	lImage *img = writeImage(val, &size);
	if(unlikely(img == NULL)){
		return lValException(lSymTypeError, "Can't serialize that", NIL);
	} else {
		lBuffer *buf = lBufferAlloc(size, true);
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
