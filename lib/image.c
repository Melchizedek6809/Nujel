/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
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

} lImageType;

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
	i32 imgSize;
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
		map->key = realloc(map->key, map->size * sizeof(i32));
		map->val = realloc(map->val, map->size * sizeof(void *));
	}
	map->key[map->len] = key;
	map->val[map->len] = val;
	map->len++;
}

static const lSymbol *readSymbol(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	(void)staticImage;
	(void)map;
	if(off < 0){return NULL;}
	return lSymS((const char *)&img->data[off]);
}

static lArray *readArray(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	if(off < 0){return NULL;}
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lArray *)mapP; }
	const u32 raw = *(u32 *)((void *)&img->data[off]);
	const i32 len = raw & 0xFFFFFF;
	const u8 flags = len >> 24;
	lArray *out = lArrayAlloc(len);
	out->flags = flags;
	readMapSet(map, off, out);
	const i32 *in = (i32 *)((void *)&img->data[off+4]);
	for(int i=0;i<len;i++){
		out->data[i] = readVal(map, img, in[i], staticImage);
	}
	return out;
}

static lBytecodeArray *readBytecodeArray(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	if(off < 0){return NULL;}
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lBytecodeArray *)mapP; }
	const i32 *in = (i32 *)((void *)&img->data[off]);
	const i32 len = *in++;
	lBytecodeArray *ret = lBytecodeArrayAllocRaw();
	readMapSet(map, off, ret);
	ret->literals = readArray(map, img, *in++, staticImage);
	if(staticImage){
		ret->data = (void *)in;
	} else {
		ret->data = malloc(len);
		memcpy(ret->data, in, len);
	}
	ret->dataEnd = &ret->data[len];
	return ret;
}

static lPair *readPair(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(off < 0){return NULL;}
	if(mapP != NULL){ return (lPair *)mapP; }
	const i32 *pair = (const i32 *)((void *)&img->data[off]);
	lPair *ret = lPairAllocRaw();
	readMapSet(map, off, ret);
	ret->car = pair[0] >= 0 ? readVal(map, img, pair[0], staticImage) : NIL;
	ret->cdr = pair[1] >= 0 ? readVal(map, img, pair[1], staticImage) : NIL;
	return ret;
}

static lBuffer *readBuffer(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lBuffer *)mapP; }
	if(off < 0){return NULL;}
	lImageBuffer *imgBuf = (lImageBuffer *)((void *)&img->data[off]);
	bool immutable = imgBuf->flags & BUFFER_IMMUTABLE;
	lBuffer *buf = lBufferAlloc(imgBuf->length, immutable);
	readMapSet(map, off, buf);
	if(staticImage && immutable){
		buf->buf = imgBuf->data;
		buf->flags |= BUFFER_STATIC;
	} else {
		buf->buf = malloc(imgBuf->length);
		memcpy(buf->buf, imgBuf->data, imgBuf->length);
	}
	return buf;
}

static lBufferView *readBufferView(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lBufferView *)mapP; }
	if(off < 0){return NULL;}
	lImageBufferView *imgBuf = (lImageBufferView *)((void *)&img->data[off]);
	lBuffer *buf = readBuffer(map, img, imgBuf->buffer, staticImage);
	lBufferView *view = lBufferViewAllocRaw();
	readMapSet(map, off, view);
	view->buf = buf;
	view->offset = imgBuf->offset;
	view->length = imgBuf->length;
	view->flags = imgBuf->flags;
	view->type = imgBuf->type;
	return view;
}

static lTree *readTree(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(off < 0){return NULL;}
	if(mapP != NULL){ return (lTree *)mapP; }
	const i32 *in = (i32 *)((void *)&img->data[off]);
	const i32 len = *in++;
	lTree *ret = lTreeAllocRaw();
	readMapSet(map, off, ret);
	for(int i=0;i<len;i++){
		const lSymbol *s = readSymbol(map, img, *in++, staticImage);
		lVal v = readVal(map, img, *in++, staticImage);
		ret = lTreeInsert(ret, s, v);
	}
	return ret;
}

static lNFunc *readNFunc(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	(void)map;
	if(off < 0){return NULL;}
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
	if(off < 0){return NULL;}
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
	if(off < 0){return NULL;}
	lImageClosure *clo = (lImageClosure *)((void *)&img->data[off]);
	lClosure *ret = lClosureAllocRaw();
	readMapSet(map, off, ret);

	ret->args = readVal(map, img, clo->args, staticImage);
	ret->data = readTree(map, img, clo->data, staticImage);
	ret->meta = readTree(map, img, clo->meta, staticImage);
	ret->parent = readClosure(map, img, clo->parent, staticImage);
	ret->sp = clo->sp;
	ret->text = readBytecodeArray(map, img, clo->text, staticImage);
	ret->type = clo->type;
	if(ret->text != NULL){
		ret->ip = &ret->text->data[clo->ip];
	} else {
		ret->ip = NULL;
	}

	return ret;
}

static lVal readVal(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	if(off < 0){return NIL;}
	const lImageType T = img->data[off++];
	const i32 *dword = (i32 *)((void *)&img->data[off]);

	switch(T){
	default:
		return lValException(lSymReadError, "Can't have rootValues of that Type", NIL);
	case litFileHandle:
		switch(img->data[off]){
		case 0:
			return lValFileHandle(stdin);
		case 1:
			return lValFileHandle(stdout);
		case 2:
			return lValFileHandle(stderr);
		default:
			return lValException(lSymReadError, "Can't serialize file handles other than stdin, stdout or stderr", NIL);
		}
	case litNativeFunc:
		return lValAlloc(ltNativeFunc, readNFunc(map, img, *dword, staticImage));
	case litType:
		return lValAlloc(ltType, readType(map, img, *dword, staticImage));
	case litBytecodeArr:
		return lValAlloc(ltBytecodeArr, readBytecodeArray(map, img, *dword, staticImage));
	case litTree:
		return lValTree(readTree(map, img, *dword, staticImage));
	case litArray:
		return lValAlloc(ltArray, readArray(map, img, *dword, staticImage));
	case litPair:
		return lValAlloc(ltPair, readPair(map, img, *dword, staticImage));
	case litBufferView:
		return lValAlloc(ltBufferView, readBufferView(map, img, *dword, staticImage));
	case litLambda:
		return lValAlloc(ltLambda, readClosure(map, img, *dword, staticImage));
	case litMacro:
		return lValAlloc(ltMacro, readClosure(map, img, *dword, staticImage));
	case litEnvironment:
		return lValAlloc(ltEnvironment, readClosure(map, img, *dword, staticImage));
	case litString:
		return lValAlloc(ltString, readBuffer(map, img, *dword, staticImage));
	case litBuffer:
		return lValAlloc(ltBuffer, readBuffer(map, img, *dword, staticImage));
	case litSymbol:
		return lValAlloc(ltSymbol, (void *)readSymbol(map, img, *dword, staticImage));
	case litKeyword:
		return lValAlloc(ltKeyword, (void *)readSymbol(map, img, *dword, staticImage));
	case litNil:
		return NIL;
	case litInt64:
		return lValInt(*(i64 *)((void *)&img->data[off]));
	case litInt32:
		return lValInt(*(i32 *)((void *)&img->data[off]));
	case litInt16:
		return lValInt(*(i16 *)((void *)&img->data[off]));
	case litInt8:
		return lValInt(*(i8 *)((void *)&img->data[off]));
	case litFloat: {
		const double *fword = (double *)((void *)&img->data[off]);
		return lValFloat(*fword);
	}
	case litFalse:
		return lValBool(false);
	case litTrue:
		return lValBool(true);
	}
	return NIL;
}

lVal readImage(const void *ptr, size_t imgSize, bool staticImage){
	const lImage *img = ptr;
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
	map.imgSize = imgSize-4;

	lVal ret = readVal(&map, img, 0, staticImage);
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
	const i32 eleSize = strLen+1;
	ctxRealloc(ctx, eleSize);


	const i32 curOff = ctx->curOff;
	writeMapSet(&ctx->map, (void *)v, curOff);
	ctx->curOff += eleSize;
	memcpy(&ctx->start[curOff], v->c, strLen);
        ctx->start[curOff + strLen] = 0;
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
	if(v == NULL){return -1;}
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const int len = lTreeSize(v);
	const i32 eleSize = 4 + (4*2*len);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(&ctx->map, (void *)v, curOff);
	i32 *out = (i32 *)((void *)&ctx->start[ctx->curOff]);
	*out = len;
	ctx->curOff += eleSize;
	ctxAddTreeVal(ctx, curOff + 4, v);
	return curOff;
}

static i32 ctxAddArray(writeImageContext *ctx, lArray *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = 4 + (4 * v->length);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(&ctx->map, (void *)v, curOff);
	ctx->curOff += eleSize;

	i32 *out = (i32 *)((void *)&ctx->start[curOff]);
	out[0] = ((u32)v->length) | (((u32)v->flags) << 24);
	for(int i=0;i<v->length;i++){
		const i32 off = ctxAddVal(ctx, v->data[i]);
		out = (i32 *)((void *)&ctx->start[curOff + 4 + i*4]);
		*out = off;
	}

	return curOff;
}

static i32 ctxAddBytecodeArray(writeImageContext *ctx, lBytecodeArray *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }
	if(v == NULL){return -1;}

	int len = v->dataEnd - v->data;
	i32 eleSize = 8 + len;
	ctxRealloc(ctx, eleSize);

	i32 curOff = ctx->curOff;
	writeMapSet(&ctx->map, (void *)v, curOff);

	ctx->curOff += eleSize;

	*((i32 *)((void *)&ctx->start[curOff])) = len;
	const i32 arr = ctxAddArray(ctx, v->literals);
	*((i32 *)((void *)&ctx->start[curOff+4])) = arr;
	memcpy(&ctx->start[curOff+8], v->data, len);

	return curOff;
}

static i32 ctxAddClosure(writeImageContext *ctx, lClosure *v){
	if(v == NULL){return -1;}
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = sizeof(lImageClosure);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(&ctx->map, (void *)v, curOff);
	ctx->curOff += eleSize;

	const i32 args = ctxAddVal(ctx, v->args);
	const i32 data = ctxAddTree(ctx, v->data);
	const i32 meta = ctxAddTree(ctx, v->meta);
	const i32 text = ctxAddBytecodeArray(ctx, v->text);
	const i32 parent = ctxAddClosure(ctx, v->parent);

	lImageClosure *out = (lImageClosure *)((void *)&ctx->start[curOff]);

	out->args = args;
	out->data = data;
	out->meta = meta;
	out->text = text;
	out->ip   = v->text == NULL ? 0 : (i32)((u8 *)v->ip - (u8 *)v->text->data);
	out->parent = parent;
	out->sp   = v->sp;
	out->type = v->type;

	return curOff;
}

static i32 ctxAddBuffer(writeImageContext *ctx, lBuffer *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = sizeof(lImageBuffer) + v->length;
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(&ctx->map, (void *)v, curOff);
	lImageBuffer *out = (lImageBuffer *)((void *)&ctx->start[ctx->curOff]);
	ctx->curOff += eleSize;

	out->flags = v->flags;
	out->length = v->length;
	memcpy(out->data, v->data, v->length);
	return curOff;
}

static i32 ctxAddBufferView(writeImageContext *ctx, lBufferView *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = sizeof(lImageBufferView);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(&ctx->map, (void *)v, curOff);
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
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = 8;
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(&ctx->map, (void *)v, curOff);
	ctx->curOff += eleSize;

	const i32 car = v->car.type != ltNil ? ctxAddVal(ctx, v->car) : -1;
	const i32 cdr = v->cdr.type != ltNil ? ctxAddVal(ctx, v->cdr) : -1;
	i32 *out = (i32 *)((void *)&ctx->start[curOff]);
	out[0] = car;
	out[1] = cdr;

	return curOff;
}

static u8 ctxAddFilehandle(FILE *fh){
	if(fh == stdin){
		return 0;
	} else if(fh == stdout){
		return 1;
	} else if(fh == stderr){
		return 2;
	} else {
		return 0xFF;
	}
}

static i32 ctxAddVal(writeImageContext *ctx, lVal v){
	ctxRealloc(ctx, 16);

	const i32 curOff = ctx->curOff;
	u8 *outb = (u8 *)((void *)&ctx->start[curOff]);

	switch(v.type){
	case ltAny:
	case ltComment:
	case ltException:
		exit(234);
	case ltFileHandle:
		ctx->curOff += 2;
		*outb++ = litFileHandle;
		*outb = ctxAddFilehandle(v.vFileHandle);
		break;
	case ltNativeFunc: {
		ctx->curOff += 5;
		*outb = litNativeFunc;
		const i32 off = ctxAddSymbol(ctx, v.vNFunc->name);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltType: {
		ctx->curOff += 5;
		*outb = litType;
		const i32 off = ctxAddSymbol(ctx, v.vType->name);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltBytecodeArr: {
		ctx->curOff += 5;
		*outb = litBytecodeArr;
		const i32 off = ctxAddBytecodeArray(ctx, v.vBytecodeArr);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltTree: {
		ctx->curOff += 5;
		*outb = litTree;
		const i32 off = ctxAddTree(ctx, v.vTree->root);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltArray: {
		ctx->curOff += 5;
		*outb = litArray;
		const i32 off = ctxAddArray(ctx, v.vArray);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltPair: {
		ctx->curOff += 5;
		*outb = litPair;
		const i32 off = ctxAddPair(ctx, v.vList);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltSymbol:
	case ltKeyword: {
		ctx->curOff += 5;
		*outb = v.type == ltSymbol ? litSymbol : litKeyword;
		const i32 off = ctxAddSymbol(ctx, v.vSymbol);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltBufferView: {
		ctx->curOff += 5;
		*outb = litBufferView;
		const i32 off = ctxAddBufferView(ctx, v.vBufferView);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltLambda: {
		ctx->curOff += 5;
		*outb = litLambda;
		const i32 off = ctxAddClosure(ctx, v.vClosure);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltMacro: {
		ctx->curOff += 5;
		*outb = litMacro;
		const i32 off = ctxAddClosure(ctx, v.vClosure);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltEnvironment: {
		ctx->curOff += 5;
		*outb = litEnvironment;
		const i32 off = ctxAddClosure(ctx, v.vClosure);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltString:
	case ltBuffer: {
		ctx->curOff += 5;
		*outb = v.type == ltString ? litString : litBuffer;
		const i32 off = ctxAddBuffer(ctx, v.vBuffer);
		i32 *outd = (i32 *)((void *)&ctx->start[curOff+1]);
		*outd = off;
		break; }
	case ltInt:
		if(v.vInt == ((i8)v.vInt)){
			ctx->curOff += 2;
			*outb++ = litInt8;
			*outb++ = v.vInt;
		} else if(v.vInt == ((i16)v.vInt)){
			*outb++ = litInt16;
			ctx->curOff += 3;
			i16 *out = (i16 *)((void *)&ctx->start[curOff+1]);
			*out = v.vInt;
		} else if(v.vInt == ((i32)v.vInt)){
			*outb++ = litInt32;
			ctx->curOff += 5;
			i32 *out = (i32 *)((void *)&ctx->start[curOff+1]);
			*out = v.vInt;
		} else {
			*outb++ = litInt64;
			ctx->curOff += 9;
			u64 *outq = (u64 *)((void *)&ctx->start[curOff+1]);
			*outq = v.vInt;
		}
		break;
	case ltFloat: {
		ctx->curOff += 9;
		*outb = litFloat;
		double *outf = (double *)((void *)&ctx->start[curOff+1]);
		*outf = v.vFloat;
		break; }
	case ltBool:
		ctx->curOff += 1;
		*outb = v.vBool ? litTrue : litFalse;
		break;
	case ltNil:
		ctx->curOff += 1;
		*outb = litNil;
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
	return readImage(val.vBuffer->buf, val.vBuffer->length, false);
}

void lOperationsImage(lClosure *c){
	lAddNativeFuncV(c,"image/serialize",   "(val)", "Serializes val into a binary representation that can be stored", lnfSerialize, 0);
	lAddNativeFuncV(c,"image/deserialize", "(buf)", "Deserializes buf into a value", lnfDeserialize, 0);
}
