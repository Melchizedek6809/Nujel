/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#include "nujel.h"
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

#include "image.h"

typedef struct {
	i32 imgSize;
	i32 curOff;
	lMap *map;
} readImageMap;

static lVal readVal(readImageMap *map, const lImage *img, i32 off, bool staticImage);

static void *readMapGet(readImageMap *map, i32 key){
	lVal v = lMapRef(map->map, lValInt(key));
	if(v.type == ltInt){
		return (void *)v.vInt;
	} else {
		return NULL;
	}
}

static void readMapSet(readImageMap *map, i32 key, void *val){
	lMapSet(map->map, lValInt(key), lValInt((u64)val));
}

static inline i32 readI8(const lImage *img, i32 off){
	return img->data[off  ];
}

static inline i32 readI16(const lImage *img, i32 off){
	return img->data[off  ] | (img->data[off+1]<<8);
}

static i32 readI24(const lImage *img, i32 off){
	return img->data[off  ] | (img->data[off+1]<<8) | (img->data[off+2]<<16);
}

static i32 readI32(const lImage *img, i32 off){
	return img->data[off  ] | (img->data[off+1]<<8) | (img->data[off+2]<<16) | (img->data[off+3]<<24);
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
	const i32 car = readI24(img, off);
	const i32 cdr = readI24(img, off+3);
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
	const u8 flags = readI8(img, off);
	lBuffer *buf = readBuffer(map, img, readI24(img, off+1), staticImage);
	lBufferView *view = lBufferViewAllocRaw();
	readMapSet(map, off, view);
	view->flags = flags;
	view->buf = buf;
	view->length = readI32(img, off+4);
	view->offset = readI32(img, off+8);
	view->type = readI8(img, off+12);
	return view;
}

static lMap *readMap(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return (lMap *)mapP; }
	if(off < 0){return NULL;}
	if(off >= (1<<24)-1){return NULL;}
	const i32 len = img->data[off] | (img->data[off+1]<<8);
	lMap *ret = lMapAllocRaw();
	readMapSet(map, off, ret);
	off += 2;
	for(int i=0;i<len;i++){
		const i32 key = readI24(img, off+0);
		const i32 val = readI24(img, off+3);
		off += 6;
		lMapSet(ret, readVal(map, img, key, staticImage), readVal(map, img, val, staticImage));
	}
	return ret;
}

static lTree *readTree(readImageMap *map, const lImage *img, i32 off, bool staticImage){
	const void *mapP = readMapGet(map, off);
	if(mapP != NULL){ return ((lTreeRoot *)mapP)->root; }
	if(off < 0){return NULL;}
	if(off >= (1<<24)-1){return NULL;}
	const i32 len = img->data[off] | (img->data[off+1]<<8);
	lTreeRoot *root = lTreeRootAllocRaw();
	readMapSet(map, off, root);
	off += 2;
	for(int i=0;i<len;i++){
		const i32 sym = readI24(img, off+0);
		const i32 val = readI24(img, off+3);
		off += 6;

		const lSymbol *s = readSymbol(map, img, sym, staticImage);
		if(s){
			lVal v = readVal(map, img, val, staticImage);
			root->root = lTreeInsert(root->root, s, v);
		}
	}
	return root->root;
}

static lNFunc *readNFunc(readImageMap *map, const lImage *img, i32 off, bool staticImage){
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
	map->curOff++;
	switch(T){
	case litFileHandle:
		map->curOff++;
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
		map->curOff += 8;
		return lValInt(*(i64 *)((void *)&img->data[off]));
	case litInt32:
		map->curOff += 4;
		return lValInt(*(i32 *)((void *)&img->data[off]));
	case litInt16:
		map->curOff += 2;
		return lValInt(*(i16 *)((void *)&img->data[off]));
	case litInt8:
		map->curOff += 1;
		return lValInt(*(i8 *)((void *)&img->data[off]));
	case litFloat: {
		map->curOff += 8;
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

	const i32 tbyte = readI24(img, off);
	map->curOff += 3;
	switch(T){
	default:
		return lValException(lSymReadError, "Can't have rootValues of that Type", NIL);
	case litNativeFunc:
		return lValAlloc(ltNativeFunc, readNFunc(map, img, tbyte, staticImage));
	case litType:
		return lValAlloc(ltType, readType(map, img, tbyte, staticImage));
	case litBytecodeArr:
		return lValAlloc(ltBytecodeArr, readBytecodeArray(map, img, tbyte, staticImage));
	case litMap:
		return lValMap(readMap(map, img, tbyte, staticImage));
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
	map.map = lMapAllocRaw();

	lVal ret = readVal(&map, img, 0, staticImage);
	return ret;
}

lVal lnfDeserialize(lVal val){
	reqBuffer(val);
	return readImage(val.vBuffer->buf, val.vBuffer->length, false);
}
