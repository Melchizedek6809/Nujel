/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

typedef struct {
	i32 imgSize;
	i32 len;
	i32 size;
	i32 *key;
	void **val;
} readImageMap;

static lVal readVal(readImageMap *map, const lImage *img, i32 off, bool staticImage);

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
	if(off >= (1<<24)-1){return NULL;}
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lArray *)mapP; }

	const i32 *in = (i32 *)((void *)&img->data[off]);
	const u32 raw = in[0];
	const i32 len = raw & 0x0FFFFFFF;
	const u8 flags = raw >> 28;
	lArray *out = lArrayAlloc(len);
	out->flags = flags;
	readMapSet(map, off, out);

	out->flags = flags;
	in++;

	for(int i=0;i<len;i++){
		out->data[i] = readVal(map, img, in[i], staticImage);
	}
	return out;
}

static lBytecodeArray *readBytecodeArray(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	if(off < 0){return NULL;}
	if(off >= (1<<24)-1){return NULL;}
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
	if(mapP != NULL){ return (lPair *)mapP; }
	if(off < 0){return NULL;}
	if(off >= (1<<24)-1){return NULL;}
	lPair *ret = lPairAllocRaw();
	readMapSet(map, off, ret);
	const i32 car = img->data[off  ] | (img->data[off+1]<<8) | (img->data[off+2]<<16);
	const i32 cdr = img->data[off+3] | (img->data[off+4]<<8) | (img->data[off+5]<<16);
	ret->car = car < ((1<<24)-1) ? readVal(map, img, car, staticImage) : NIL;
	ret->cdr = cdr < ((1<<24)-1) ? readVal(map, img, cdr, staticImage) : NIL;
	return ret;
}

static lBuffer *readBuffer(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lBuffer *)mapP; }
	if(off < 0){return NULL;}
	if(off >= (1<<24)-1){return NULL;}
	const u32 raw = *(u32 *)((void *)&img->data[off]);
	const u32 len = raw & 0x0FFFFFFF;
	const bool immutable = (raw >> 28) & BUFFER_IMMUTABLE;
	void *data = (void *)&img->data[off+4];
	lBuffer *buf = lBufferAlloc(len, immutable);
	readMapSet(map, off, buf);
	if(staticImage && immutable){
		buf->buf = data;
		buf->flags |= BUFFER_STATIC;
	} else {
		buf->buf = malloc(len);
		memcpy(buf->buf, data, len);
	}
	return buf;
}

static lBufferView *readBufferView(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lBufferView *)mapP; }
	if(off < 0){return NULL;}
	if(off >= (1<<24)-1){return NULL;}
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
	if(mapP != NULL){ return (lTree *)mapP; }
	if(off < 0){return NULL;}
	if(off >= (1<<24)-1){return NULL;}
	const i32 len = img->data[off] | (img->data[off+1]<<8);
	lTree *ret = lTreeAllocRaw();
	readMapSet(map, off, ret);
	off += 2;
	for(int i=0;i<len;i++){
		const i32 sym = img->data[off+0] | (img->data[off+1] << 8) | (img->data[off+2] << 16);
		const i32 val = img->data[off+3] | (img->data[off+4] << 8) | (img->data[off+5] << 16);
		off += 6;

		const lSymbol *s = readSymbol(map, img, sym, staticImage);
		lVal v = readVal(map, img, val, staticImage);
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
	if(off >= (1<<24)-1){return NULL;}
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
	if(off >= (1<<24)-1){return NULL;}
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
	switch(T){
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
	default:
		break;
	}

	const i32 tbyte = img->data[off] | (img->data[off+1] << 8) | (img->data[off+2] << 16);
	switch(T){
	default:
		return lValException(lSymReadError, "Can't have rootValues of that Type", NIL);
	case litNativeFunc:
		return lValAlloc(ltNativeFunc, readNFunc(map, img, tbyte, staticImage));
	case litType:
		return lValAlloc(ltType, readType(map, img, tbyte, staticImage));
	case litBytecodeArr:
		return lValAlloc(ltBytecodeArr, readBytecodeArray(map, img, tbyte, staticImage));
	case litTree:
		return lValTree(readTree(map, img, tbyte, staticImage));
	case litArray:
		return lValAlloc(ltArray, readArray(map, img, tbyte, staticImage));
	case litPair:
		return lValAlloc(ltPair, readPair(map, img, tbyte, staticImage));
	case litBufferView:
		return lValAlloc(ltBufferView, readBufferView(map, img, tbyte, staticImage));
	case litLambda:
		return lValAlloc(ltLambda, readClosure(map, img, tbyte, staticImage));
	case litMacro:
		return lValAlloc(ltMacro, readClosure(map, img, tbyte, staticImage));
	case litEnvironment:
		return lValAlloc(ltEnvironment, readClosure(map, img, tbyte, staticImage));
	case litString:
		return lValAlloc(ltString, readBuffer(map, img, tbyte, staticImage));
	case litBuffer:
		return lValAlloc(ltBuffer, readBuffer(map, img, tbyte, staticImage));
	case litSymbol:
		return lValAlloc(ltSymbol, (void *)readSymbol(map, img, tbyte, staticImage));
	case litKeyword:
		return lValAlloc(ltKeyword, (void *)readSymbol(map, img, tbyte, staticImage));
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

lVal lnfDeserialize(lVal val){
	reqBuffer(val);
	return readImage(val.vBuffer->buf, val.vBuffer->length, false);
}
