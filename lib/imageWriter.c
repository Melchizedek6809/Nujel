/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

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
	if((v == NULL) || (v->key == NULL)){return curOff;}

	const i32 sym = ctxAddSymbol(ctx, v->key);
	ctx->start[curOff]   = (sym    )&0xFF;
	ctx->start[curOff+1] = (sym>> 8)&0xFF;
	ctx->start[curOff+2] = (sym>>16)&0xFF;

	const i32 val = ctxAddVal(ctx, v->value);
	ctx->start[curOff+3] = (val    )&0xFF;
	ctx->start[curOff+4] = (val>> 8)&0xFF;
	ctx->start[curOff+5] = (val>>16)&0xFF;

	curOff += 6;
	curOff = ctxAddTreeVal(ctx, curOff, v->left);
	curOff = ctxAddTreeVal(ctx, curOff, v->right);
	return curOff;
}

static i32 ctxAddTree(writeImageContext *ctx, lTree *v){
	if(v == NULL){return -1;}
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const int len = lTreeSize(v);
	const i32 eleSize = 2 + (6*len);
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(&ctx->map, (void *)v, curOff);
	ctx->start[curOff  ] =  len     & 0xFF;
	ctx->start[curOff+1] = (len>>8) & 0xFF;
	ctx->curOff += eleSize;
	ctxAddTreeVal(ctx, curOff + 2, v);
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
	out[0] = (v->length & 0x0FFFFFFF) | (v->flags << 28);
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
	out->parent = parent;
	out->ip   = v->text == NULL ? 0 : (i32)((u8 *)v->ip - (u8 *)v->text->data);
	out->sp   = v->sp;
	out->type = v->type;

	return curOff;
}

static i32 ctxAddBuffer(writeImageContext *ctx, lBuffer *v){
	const i32 mapOff = writeMapGet(&ctx->map, (void *)v);
	if(mapOff > 0){ return mapOff; }

	const i32 eleSize = 4 + v->length;
	ctxRealloc(ctx, eleSize);

	const i32 curOff = ctx->curOff;
	writeMapSet(&ctx->map, (void *)v, curOff);
	u32 *out = (u32 *)((void *)&ctx->start[ctx->curOff]);
	const u32 outVal = (v->length & 0x0FFFFFFF) | (v->flags << 28);
	*out = outVal;
	ctx->curOff += eleSize;

	memcpy(&out[1], v->data, v->length);
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
	ctx->start[curOff  ] =  car      & 0xFF;
	ctx->start[curOff+1] = (car>> 8) & 0xFF;
	ctx->start[curOff+2] = (car>>16) & 0xFF;

	const i32 cdr = v->cdr.type != ltNil ? ctxAddVal(ctx, v->cdr) : -1;
	ctx->start[curOff+3] =  cdr      & 0xFF;
	ctx->start[curOff+4] = (cdr>> 8) & 0xFF;
	ctx->start[curOff+5] = (cdr>>16) & 0xFF;

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
		ctx->curOff += 4;
		*outb = litNativeFunc;
		const i32 off = ctxAddSymbol(ctx, v.vNFunc->name);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltType: {
		ctx->curOff += 4;
		*outb = litType;
		const i32 off = ctxAddSymbol(ctx, v.vType->name);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltBytecodeArr: {
		ctx->curOff += 4;
		*outb = litBytecodeArr;
		const i32 off = ctxAddBytecodeArray(ctx, v.vBytecodeArr);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltTree: {
		ctx->curOff += 4;
		*outb = litTree;
		const i32 off = ctxAddTree(ctx, v.vTree->root);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltArray: {
		ctx->curOff += 5;
		*outb = litArray;
		const i32 off = ctxAddArray(ctx, v.vArray);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltPair: {
		ctx->curOff += 4;
		*outb = litPair;
		const i32 off = ctxAddPair(ctx, v.vList);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltSymbol:
	case ltKeyword: {
		ctx->curOff += 4;
		*outb = v.type == ltSymbol ? litSymbol : litKeyword;
		const i32 off = ctxAddSymbol(ctx, v.vSymbol);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltBufferView: {
		ctx->curOff += 4;
		*outb = litBufferView;
		const i32 off = ctxAddBufferView(ctx, v.vBufferView);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltLambda: {
		ctx->curOff += 4;
		*outb = litLambda;
		const i32 off = ctxAddClosure(ctx, v.vClosure);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltMacro: {
		ctx->curOff += 4;
		*outb = litMacro;
		const i32 off = ctxAddClosure(ctx, v.vClosure);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltEnvironment: {
		ctx->curOff += 4;
		*outb = litEnvironment;
		const i32 off = ctxAddClosure(ctx, v.vClosure);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
		break; }
	case ltString:
	case ltBuffer: {
		ctx->curOff += 4;
		*outb = v.type == ltString ? litString : litBuffer;
		const i32 off = ctxAddBuffer(ctx, v.vBuffer);
		outb = ((void *)&ctx->start[curOff+1]);
		*outb++ = off&0xFF;
		*outb++ = (off>>8)&0xFF;
		*outb++ = (off>>16)&0xFF;
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
