/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "../nujel-private.h"
#endif

#include <stdlib.h>
#include <string.h>

void *lBufferData (lBuffer *v){
	return v == NULL ? NULL : v->buf;
}

const char *lStringData(const lString *v){
	return v == NULL ? NULL : v->data;
}

size_t lBufferLength(const lBuffer *v){
	return v == NULL ? 0 : v->length;
}

void *lBufferViewData(lBufferView *v){
	return &((u8 *)v->buf->buf)[v->offset * lBufferViewTypeSize(v->type)];
}

size_t lBufferViewLength(const lBufferView *v){
	return v->length * lBufferViewTypeSize(v->type);
}

static lVal *lnfBufferAllocate(lClosure *c, lVal *v) {
	const i64 size = requireNaturalInt(c, lCar(v));
	lBuffer *buf = lBufferAlloc(size, false);
	lVal *ret = lValAlloc(ltBuffer);
	ret->vBuffer = buf;
	return ret;
}

static lVal *lnfBufferLengthGet(lClosure *c, lVal *v){
	lBuffer *buf = requireBuffer(c, lCar(v));
	return lValInt(buf->length);
}

static lVal *lnfBufferLengthSet(lClosure *c, lVal *v){
	lVal *bufv = lCar(v);
	lBuffer *buf = requireMutableBuffer(c, bufv);
	const int length = requireNaturalInt(c, lCadr(v));
	if(length < buf->length){
		lExceptionThrowValClo("error", "Buffers can only grow, not shrink.", v, c);
	}
	void *nBuf = realloc(buf->buf, length);
	if (unlikely(nBuf == NULL)) {
		lExceptionThrowValClo("out-of-memory", "[buffer/length!] couldn't allocate its buffer", v, c);
		return NULL;
	}
	buf->buf = nBuf;
	buf->length = length;
	return bufv;
}

static lVal *lnfBufferImmutableGet(lClosure *c, lVal *v){
	lBuffer *buf = requireBuffer(c, lCar(v));
	return lValBool(buf->flags & BUFFER_IMMUTABLE);
}

static lVal *bufferFromPointer(lClosure *c, bool immutable, const void *data, size_t length){
	(void)c;
	lBuffer *retBuf = lBufferAllocRaw();
	retBuf->length = length;
	retBuf->flags = immutable ? BUFFER_IMMUTABLE : 0;
	retBuf->buf = malloc(length);
	if (unlikely(retBuf->buf == NULL)) {
		lExceptionThrowValClo("out-of-memory", "[buffer/length!] couldn't allocate its buffer", NULL, c);
		return NULL;
	}
	memcpy(retBuf->buf, data, length);

	lVal *retV = lValAlloc(ltBuffer);
	retV->vBuffer = retBuf;
	return retV;
}

static lVal *lnfBufferCopy(lClosure *c, lVal *v){
	lBuffer *buf = requireBuffer(c, lCar(v));
	return bufferFromPointer(c, castToBool(lCadr(v)), buf->buf, buf->length);
}

static lVal *lnfStringToBuffer(lClosure *c, lVal *v){
	lString *str = requireString(c, lCar(v));
	return bufferFromPointer(c, castToBool(lCadr(v)), str->data, str->length);
}

static lVal *lnfBufferToString(lClosure *c, lVal *v){
	lBuffer *buf = requireBuffer(c, lCar(v));
	return lValStringLen(buf->buf, buf->length);
}

static lVal *bufferView(lClosure *c, lVal *v, lBufferViewType T){
	lBuffer *buf = requireBuffer(c, lCar(v));
	const bool immutable = lCdr(v) ? castToBool(lCadr(v)) : buf->flags & BUFFER_IMMUTABLE;
	if(!immutable && (buf->flags & BUFFER_IMMUTABLE)){
		lExceptionThrowValClo("type-error", "Can't create a mutable view for an immutable buffer", v, c);
	}
	const size_t length = buf->length / lBufferViewTypeSize(T);
	lBufferView *bufView = lBufferViewAlloc(buf, T, 0, length, immutable);
	lVal *ret = lValAlloc(ltBufferView);
	ret->vBufferView = bufView;
	return ret;
}

static lVal  *lnfBufferViewU8(lClosure *c, lVal *v){ return bufferView(c, v,  lbvtU8); }
static lVal  *lnfBufferViewS8(lClosure *c, lVal *v){ return bufferView(c, v,  lbvtS8); }
static lVal *lnfBufferViewU16(lClosure *c, lVal *v){ return bufferView(c, v, lbvtU16); }
static lVal *lnfBufferViewS16(lClosure *c, lVal *v){ return bufferView(c, v, lbvtS16); }
static lVal *lnfBufferViewU32(lClosure *c, lVal *v){ return bufferView(c, v, lbvtU32); }
static lVal *lnfBufferViewS32(lClosure *c, lVal *v){ return bufferView(c, v, lbvtS32); }
static lVal *lnfBufferViewF32(lClosure *c, lVal *v){ return bufferView(c, v, lbvtF32); }
static lVal *lnfBufferViewS64(lClosure *c, lVal *v){ return bufferView(c, v, lbvtS64); }
static lVal *lnfBufferViewF64(lClosure *c, lVal *v){ return bufferView(c, v, lbvtF64); }

static lVal *lnfBufferViewRef(lClosure *c, lVal *v){
	const lBufferView *view  = requireBufferView(c, lCar(v));
	const size_t i = requireNaturalInt(c, lCadr(v));
	if(i >= view->length){
		lExceptionThrowValClo("out-of-bounds","[buffer/view/ref] index provided is out of bounds", v, c);
	}
	switch(view->type){
	default:
		epf("Unknown buffer-view type\n");
		exit(5);
		return NULL;
	case lbvtU8:
		return lValInt(((u8 *)view->buf->buf)[i]);
	case lbvtS8:
		return lValInt(((i8 *)view->buf->buf)[i]);
	case lbvtU16:
		return lValInt(((u16 *)view->buf->buf)[i]);
	case lbvtS16:
		return lValInt(((i16 *)view->buf->buf)[i]);
	case lbvtU32:
		return lValInt(((u32 *)view->buf->buf)[i]);
	case lbvtS32:
		return lValInt(((i32 *)view->buf->buf)[i]);
	case lbvtS64:
		return lValInt(((i64 *)view->buf->buf)[i]);
	case lbvtF32:
		return lValFloat(((float *)view->buf->buf)[i]);
	case lbvtF64:
		return lValFloat(((double *)view->buf->buf)[i]);
	}
}

static lVal *lnfBufferViewSet(lClosure *c, lVal *v){
	lVal *car = lCar(v);
	const lBufferView *view  = requireMutableBufferView(c, car);
	const size_t i = requireNaturalInt(c, lCadr(v));
	if(i >= view->length){
		lExceptionThrowValClo("out-of-bounds","[buffer/view/set!] index provided is out of bounds", v, c);
	}
	switch(view->type){
	default:
		epf("Unknown buffer-view type\n");
		exit(5);
	case lbvtU8:
		((u8 *)view->buf->buf)[i] = requireInt(c, lCaddr(v));
		break;
	case lbvtS8:
		((i8 *)view->buf->buf)[i] = requireInt(c, lCaddr(v));
		break;
	case lbvtS16:
		((i16 *)view->buf->buf)[i] = requireInt(c, lCaddr(v));
		break;
	case lbvtU16:
		((u16 *)view->buf->buf)[i] = requireInt(c, lCaddr(v));
		break;
	case lbvtS32:
		((i32 *)view->buf->buf)[i] = requireInt(c, lCaddr(v));
		break;
	case lbvtU32:
		((u32 *)view->buf->buf)[i] = requireInt(c, lCaddr(v));
		break;
	case lbvtS64:
		((i64 *)view->buf->buf)[i] = requireInt(c, lCaddr(v));
		break;
	case lbvtF32:
		((float *)view->buf->buf)[i] = requireFloat(c, lCaddr(v));
		break;
	case lbvtF64:
		((double *)view->buf->buf)[i] = requireFloat(c, lCaddr(v));
		break;
	}
	return car;
}

static lVal *lnfBufferViewBuffer(lClosure *c, lVal *v){
	lBufferView *view = requireBufferView(c, lCar(v));
	lVal *ret = lValAlloc(ltBuffer);
	ret->vBuffer = view->buf;
	return ret;
}

static lVal *lnfBufferViewImmutableGet(lClosure *c, lVal *v){
	return lValBool(requireBufferView(c, lCar(v))->flags & BUFFER_VIEW_IMMUTABLE);
}


void lOperationsBuffer(lClosure *c){
	lAddNativeFunc(c, "buffer/allocate",        "[length]",         "Allocate a new buffer of LENGTH",   lnfBufferAllocate);
	lAddNativeFunc(c, "buffer/length",          "[buf]",            "Return the size of BUF in bytes",   lnfBufferLengthGet);
	lAddNativeFunc(c, "buffer/length!",         "[buf new-length]", "Set the size of BUF to NEW-LENGTH", lnfBufferLengthSet);
	lAddNativeFunc(c, "buffer/immutable?",      "[buf]",            "Return #t if BUF is immutable",     lnfBufferImmutableGet);
	lAddNativeFunc(c, "buffer/copy",            "[buf immutable?]", "Return a copy of BUF that might be IMMUTABLE", lnfBufferCopy);

	lAddNativeFunc(c, "string->buffer",         "[str immutable?]", "Copy STR into a buffer and return it", lnfStringToBuffer);
	lAddNativeFunc(c, "buffer->string",         "[buf]",            "Turn BUF into a string", lnfBufferToString);

	lAddNativeFunc(c, "buffer/view/u8*",        "[buf immutable?]", "Create a new view for BUF spanning the entire area", lnfBufferViewU8);
	lAddNativeFunc(c, "buffer/view/s8*",        "[buf immutable?]", "Create a new view for BUF spanning the entire area", lnfBufferViewS8);
	lAddNativeFunc(c, "buffer/view/u16*",       "[buf immutable?]", "Create a new view for BUF spanning the entire area", lnfBufferViewU16);
	lAddNativeFunc(c, "buffer/view/s16*",       "[buf immutable?]", "Create a new view for BUF spanning the entire area", lnfBufferViewS16);
	lAddNativeFunc(c, "buffer/view/u32*",       "[buf immutable?]", "Create a new view for BUF spanning the entire area", lnfBufferViewU32);
	lAddNativeFunc(c, "buffer/view/s32*",       "[buf immutable?]", "Create a new view for BUF spanning the entire area", lnfBufferViewS32);
	lAddNativeFunc(c, "buffer/view/f32*",       "[buf immutable?]", "Create a new view for BUF spanning the entire area", lnfBufferViewF32);
	lAddNativeFunc(c, "buffer/view/s64*",       "[buf immutable?]", "Create a new view for BUF spanning the entire area", lnfBufferViewS64);
	lAddNativeFunc(c, "buffer/view/f64*",       "[buf immutable?]", "Create a new view for BUF spanning the entire area", lnfBufferViewF64);

	lAddNativeFunc(c, "buffer/view/buffer",     "[view]",           "Return the buffer of VIEW", lnfBufferViewBuffer);
	lAddNativeFunc(c, "buffer/view/immutable?", "[view]",           "Return if VIEW is immutable or not", lnfBufferViewImmutableGet);
	lAddNativeFunc(c, "buffer/view/ref",        "[view off]",       "Return the value in VIEW at OFF", lnfBufferViewRef);
	lAddNativeFunc(c, "buffer/view/set!",       "[view off val]",   "Set the value of VIEW at OFF to VAL", lnfBufferViewSet);
}
