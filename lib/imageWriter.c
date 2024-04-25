/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include "image.h"

typedef struct {
	u8 *start;
	i32 curOff;
	i32 size;
	lMap *map;
} writeImageContext;

static void writeI16(writeImageContext *ctx, i32 curOff, i32 v){
	ctx->start[curOff  ] = (v    )&0xFF;
	ctx->start[curOff+1] = (v>> 8)&0xFF;
}

static void writeI24(writeImageContext *ctx, i32 curOff, i32 v){
	ctx->start[curOff  ] = (v    )&0xFF;
	ctx->start[curOff+1] = (v>> 8)&0xFF;
	ctx->start[curOff+2] = (v>>16)&0xFF;
}

static i32 ctxAddVal(writeImageContext *ctx, lVal v);

static i32 writeMapGet(lMap *map, void *key){
	lVal v = lMapRef(map, lValInt((u64)key));
	if(v.type == ltInt){
		return v.vInt;
	} else {
		return 0;
	}
}

static void writeMapSet(lMap *map, void *key, i32 val){
	lMapSet(map, lValInt((u64)key), lValInt(val));
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
	if(v == NULL){return -1;}
	const i32 mapOff = writeMapGet(ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 strLen = strnlen(v->c, sizeof(v->c));
	const i32 eleSize = strLen+1;
	ctxRealloc(ctx, eleSize);


	const i32 curOff = ctx->curOff;
	writeMapSet(ctx->map, (void *)v, curOff);
	ctx->curOff += eleSize;
	memcpy(&ctx->start[curOff], v->c, strLen);
        ctx->start[curOff + strLen] = 0;
	return curOff;
}

static i32 ctxAddMap(writeImageContext *ctx, lMap *v){
	if(v == NULL){return -1;}
	const i32 mapOff = writeMapGet(ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const int len = v->length;
	const i32 eleSize = 2 + (6*len);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(ctx->map, (void *)v, curOff);
	writeI16(ctx, curOff, len);
	ctx->curOff += eleSize;
	i32 off = curOff + 2;
	for(uint i=0;i<v->size;i++){
		if(v->entries[i].key.type == ltNil){continue;}
		writeI24(ctx, off  , ctxAddVal(ctx, v->entries[i].key));
		writeI24(ctx, off+3, ctxAddVal(ctx, v->entries[i].val));
		off += 6;
	}
	return curOff;
}
static i32 ctxAddTreeVal(writeImageContext *ctx, i32 curOff, lTree *v){
	if((v == NULL) || (v->key == NULL)){return curOff;}

	writeI24(ctx, curOff  , ctxAddSymbol(ctx, v->key));
	writeI24(ctx, curOff+3, ctxAddVal(ctx, v->value));

	curOff += 6;
	curOff = ctxAddTreeVal(ctx, curOff, v->left);
	curOff = ctxAddTreeVal(ctx, curOff, v->right);
	return curOff;
}

static i32 ctxAddTree(writeImageContext *ctx, lTree *v){
	if(v == NULL){return -1;}
	const i32 mapOff = writeMapGet(ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const int len = lTreeSize(v);
	const i32 eleSize = 2 + (6*len);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(ctx->map, (void *)v, curOff);
	writeI16(ctx, curOff, len);
	ctx->curOff += eleSize;
	ctxAddTreeVal(ctx, curOff + 2, v);
	return curOff;
}

static i32 ctxAddArray(writeImageContext *ctx, lArray *v){
	const i32 mapOff = writeMapGet(ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = 4 + (4 * v->length);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(ctx->map, (void *)v, curOff);
	ctx->curOff += eleSize;

	i32 *out = (i32 *)((void *)&ctx->start[curOff]);
	out[0] = (v->length & 0x0FFFFFFF) | (v->flags << 28);
	for(int i=0;i<v->length;i++){
		const i32 off = ctxAddVal(ctx, v->data[i]);
		out = (i32 *)((void *)&ctx->start[curOff + 4 + i*4]);
		*out = off;
	}

	return curOff;
}

static i32 ctxAddBytecodeArray(writeImageContext *ctx, lBytecodeArray *v){
	const i32 mapOff = writeMapGet(ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }
	if(v == NULL){return -1;}

	int len = v->dataEnd - v->data;
	i32 eleSize = 8 + len;
	ctxRealloc(ctx, eleSize);

	i32 curOff = ctx->curOff;
	writeMapSet(ctx->map, (void *)v, curOff);

	ctx->curOff += eleSize;

	*((i32 *)((void *)&ctx->start[curOff])) = len;
	const i32 arr = ctxAddArray(ctx, v->literals);
	*((i32 *)((void *)&ctx->start[curOff+4])) = arr;
	memcpy(&ctx->start[curOff+8], v->data, len);

	return curOff;
}

static i32 ctxAddClosure(writeImageContext *ctx, lClosure *v){
	if(v == NULL){return -1;}
	const i32 mapOff = writeMapGet(ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = sizeof(lImageClosure);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(ctx->map, (void *)v, curOff);
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
	out->parent = parent;
	out->ip   = v->text == NULL ? 0 : (i32)((u8 *)v->ip - (u8 *)v->text->data);
	out->sp   = v->sp;
	out->type = v->type;

	return curOff;
}

static i32 ctxAddBuffer(writeImageContext *ctx, lBuffer *v){
	const i32 mapOff = writeMapGet(ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = 4 + v->length;
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(ctx->map, (void *)v, curOff);
	u32 *out = (u32 *)((void *)&ctx->start[ctx->curOff]);
	const u32 outVal = (v->length & 0x0FFFFFFF) | (v->flags << 28);
	*out = outVal;
	ctx->curOff += eleSize;

	memcpy(&out[1], v->data, v->length);
	return curOff;
}

static i32 ctxAddBufferView(writeImageContext *ctx, lBufferView *v){
	const i32 mapOff = writeMapGet(ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = sizeof(lImageBufferView);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(ctx->map, (void *)v, curOff);
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
	const i32 mapOff = writeMapGet(ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = 8;
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(ctx->map, (void *)v, curOff);
	ctx->curOff += eleSize;

	const i32 car = v->car.type != ltNil ? ctxAddVal(ctx, v->car) : -1;
	writeI24(ctx, curOff, car);

	const i32 cdr = v->cdr.type != ltNil ? ctxAddVal(ctx, v->cdr) : -1;
	writeI24(ctx, curOff+3, cdr);

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
	case ltNativeFunc:
		ctx->curOff += 4;
		*outb = litNativeFunc;
		writeI24(ctx, curOff+1, ctxAddSymbol(ctx, v.vNFunc->name));
		break;
	case ltType:
		ctx->curOff += 4;
		*outb = litType;
		writeI24(ctx, curOff+1, ctxAddSymbol(ctx, v.vType->name));
		break;
	case ltBytecodeArr:
		ctx->curOff += 4;
		*outb = litBytecodeArr;
		writeI24(ctx, curOff+1, ctxAddBytecodeArray(ctx, v.vBytecodeArr));
		break;
	case ltTree:
		ctx->curOff += 4;
		*outb = litTree;
		writeI24(ctx, curOff+1, ctxAddTree(ctx, v.vTree->root));
		break;
	case ltMap:
		ctx->curOff += 4;
		*outb = litMap;
		writeI24(ctx, curOff+1, ctxAddMap(ctx, v.vMap));
		break;
	case ltArray:
		ctx->curOff += 5;
		*outb = litArray;
		writeI24(ctx, curOff+1, ctxAddArray(ctx, v.vArray));
		break;
	case ltPair:
		ctx->curOff += 4;
		*outb = litPair;
		writeI24(ctx, curOff+1, ctxAddPair(ctx, v.vList));
		break;
	case ltSymbol:
	case ltKeyword:
		ctx->curOff += 4;
		*outb = v.type == ltSymbol ? litSymbol : litKeyword;
		writeI24(ctx, curOff+1, ctxAddSymbol(ctx, v.vSymbol));
		break;
	case ltBufferView:
		ctx->curOff += 4;
		*outb = litBufferView;
		writeI24(ctx, curOff+1, ctxAddBufferView(ctx, v.vBufferView));
		break;
	case ltLambda:
		ctx->curOff += 4;
		*outb = litLambda;
		writeI24(ctx, curOff+1, ctxAddClosure(ctx, v.vClosure));
		break;
	case ltMacro:
		ctx->curOff += 4;
		*outb = litMacro;
		writeI24(ctx, curOff+1, ctxAddClosure(ctx, v.vClosure));
		break;
	case ltEnvironment:
		ctx->curOff += 4;
		*outb = litEnvironment;
		writeI24(ctx, curOff+1, ctxAddClosure(ctx, v.vClosure));
		break;
	case ltString:
	case ltBuffer:
		ctx->curOff += 4;
		*outb = v.type == ltString ? litString : litBuffer;
		writeI24(ctx, curOff+1, ctxAddBuffer(ctx, v.vBuffer));
		break;
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
	ctx.map = lMapAllocRaw();
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
	return buf;
}

lVal lnfSerialize(lVal val){
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
